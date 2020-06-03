//
// Created by muhzi on 6/1/20.
//

#include "analyzer.h"
#include "ast.h"
#include "value.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>


static VarList * varlist_new (char *name, int stmt_no, int scope);
static int varlist_is_equal (VarList *l1, VarList *l2);
static void varlist_add (VarList **dst, VarList *src, int cur_stmt_scope);
static void varlist_unique_add_from_reflist (VarList **vll, RefList *rl, int stmt_no);
static void varlist_add_node (VarList **vll, char *name, int stmt_no, int scope);
static void varlist_delete_node (VarList **vll, VarList *node);
static void varlist_free (VarList *vl);

static void print_ref (RefList *ref);
static void pretty_print_reaching_defs_table (VarList **IN, VarList **OUT, int ast_sz);

void
analyze (AST* t) {
    //process_propagated_constants(t);
    process_reaching_definitions(t);
}

void
process_reaching_definitions (AST *t)
{
    int ast_sz = ast_get_size(t);

    // IN & OUT tables for each statement.
    VarList *IN[ast_sz], *OUT[ast_sz];

    // First iteration - initialization.
    AST *p = t;
    for (int i = 0; i < ast_sz; i++) {
        IN[i] = NULL;
        OUT[i] = NULL;
        varlist_unique_add_from_reflist(&OUT[i], p->deflist, i);

        p = p->next;
    }

    printf("*** Reaching definitions ***\n\n");

    int cnt = 0;
    int recently_updated = 1;
    while (recently_updated) {
        recently_updated = 0;

        AST *p = t;
        int branch_off_node, branch_off_dist;
        for (int i = 0; i < ast_sz; i++, p = p->next) {
            if (p->type == NODE_TYPE_ITER_STMT || p->type == NODE_TYPE_IF_STMT) {
                branch_off_dist = p->num_of_statements+1;
                branch_off_node = i+branch_off_dist;
            }

            // IN = PREVIOUS OUT
            VarList *old_in = IN[i];
            IN[i] = NULL;
            if (i > 0)
                varlist_add(&IN[i], OUT[i - 1], p->scope);
            if (p->type == NODE_TYPE_ITER_STMT && i+p->num_of_statements < ast_sz)
                varlist_add(&IN[i], OUT[i + branch_off_dist - 1], p->scope);
            if (i == branch_off_node)
                varlist_add(&IN[i], OUT[i - branch_off_dist], p->scope);

            // OUT = IN + DEFINE - KILL
            VarList *old_out = OUT[i];
            OUT[i] = NULL;
            varlist_add(&OUT[i], IN[i], p->scope);
            varlist_unique_add_from_reflist(&OUT[i], p->deflist, i);

            if (!varlist_is_equal(old_in, IN[i]))
                recently_updated = 1;
            if (!varlist_is_equal(old_out, OUT[i]))
                recently_updated = 1;

            varlist_free(old_in);
            varlist_free(old_out);
        }

        if (cnt++ > 0)
            printf("\n");
        printf(">>> Iteration no. %d\n", cnt);
        pretty_print_reaching_defs_table(&IN[0], &OUT[0], ast_sz);
    }
}

void
process_propagated_constants (AST *t)
{
    printf("*** Printing out propagated values of variables ***\n");

    RefList *whitelist = NULL;
    for (int i = 0; t != NULL; t = t->next, i++) {
        printf("\tstmt %d: ", i);

        RefList *dl = t->deflist;
        for (; dl != NULL; dl = dl->next) {
            if (t->type == NODE_TYPE_ITER_STMT) {
                RefList *wref = reflist_new(dl->sym);
                wref->next = whitelist;
                whitelist = wref;

                continue;
            }

            if (reflist_has_sym(whitelist, dl->sym)) {
                // Delete the node.
                if (whitelist != NULL) {
                    for (RefList *wl = whitelist; wl->next != NULL; wl = wl->next) {
                        if (wl->next->sym == dl->sym)
                            wl->next = wl->next->next;
                    }
                }
            }

            print_ref(dl);
            printf(" ");
        }

        RefList *rl = t->reflist;
        for (; rl != NULL; rl = rl->next) {
            if (reflist_has_sym(whitelist, rl->sym))
                rl->vr = NO_VALUE_RECORD;

            print_ref(rl);
            printf(" ");
        }

        printf("\n");
    }

    printf("\n");
}

static VarList *
varlist_new (char *name, int stmt_no, int scope)
{
    VarList *new_node = malloc(sizeof(VarList));
    new_node->name = name;
    new_node->stmt_no = stmt_no;
    new_node->scope = scope;
    new_node->next = NULL;
    new_node->prev = NULL;
    return new_node;
}

static int
varlist_is_equal (VarList *l1, VarList *l2)
{
    for (; l1 != NULL; l1 = l1->next, l2 = l2->next) {
        if (l2 == NULL)
            return 0;

        if (strcmp(l1->name, l2->name) != 0 || l1->stmt_no != l2->stmt_no)
            return 0;
    }

    if (l2 != NULL)
        return 0;

    return 1;
}

static void
varlist_add (VarList **dst, VarList *src, int cur_stmt_scope)
{
    for (; src != NULL; src = src->next) {
        if (src->scope > cur_stmt_scope)
            continue;

        int already_exists = 0;
        for (VarList *p = *dst; p != NULL; p = p->next) {
            if (strcmp(p->name, src->name) == 0 && p->stmt_no == src->stmt_no) {
                already_exists = 1;
                break;
            }
        }

        if (!already_exists)
            varlist_add_node(dst, src->name, src->stmt_no, src->scope);
    }
}

static void
varlist_unique_add_from_reflist (VarList **vll, RefList *rl, int stmt_no)
{
    for (; rl != NULL; rl = rl->next) {
        int already_exists = 0;
        for (VarList *p = *vll; p != NULL; p = p->next) {
            if (strcmp(p->name, rl->sym->s_name) == 0 && p->stmt_no == stmt_no) {
                already_exists = 1;
                break;
            } else if (strcmp(p->name, rl->sym->s_name) == 0 && p->stmt_no != stmt_no) {
                varlist_delete_node(vll, p);
            }
        }

        if (!already_exists)
            varlist_add_node(vll, rl->sym->s_name, stmt_no, rl->sym->scope);
    }
}

static void
varlist_add_node (VarList **vll, char *name, int stmt_no, int scope)
{
    VarList *new_node = varlist_new(name, stmt_no, scope);
    new_node->next = *vll;

    if (*vll != NULL)
        (*vll)->prev = new_node;

    *vll = new_node;
}

static void
varlist_delete_node (VarList **vll, VarList *node)
{
    if (node->prev != NULL)
        node->prev->next = node->next;
    else
        *vll = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

static void
varlist_free (VarList *vl)
{
    while (vl != NULL) {
        VarList *tmp = vl;
        vl = vl->next;
        free(tmp);
    }
}

static void
print_ref(RefList *ref)
{
    double value = value_record_get_value(ref->vr);
    switch (ref->vr.type) {
        case SYMVAL_TYPE_UNKNOWN:
            printf("%s(UNKNOWN)", ref->sym->s_name);
            break;
        case SYMVAL_TYPE_INTEGER:
            printf("%s(%d)", ref->sym->s_name, (int)value);
            break;
        case SYMVAL_TYPE_FLOAT:
            printf("%s(%f)", ref->sym->s_name, value);
            break;
        case SYMVAL_TYPE_CHAR:
            printf("%s(%c)", ref->sym->s_name, (char)value);
            break;
    }
}

void
pretty_print_reaching_defs_table (VarList **IN, VarList **OUT, int ast_sz)
{
    const int COLUMN_LENGTH = 70;

    for (int i = 0; i < COLUMN_LENGTH; i++)
        printf("-");
    printf("\n");

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < COLUMN_LENGTH / 4; j++)
            printf(" ");

        if (i == 0)
            printf("IN");
        else
            printf("OUT");

        for (int j = 0; j < COLUMN_LENGTH / 4; j++)
            printf(" ");

        if (i == 0)
            printf("|");
    }
    printf("\n");

    for (int i = 0; i < COLUMN_LENGTH; i++)
        printf("-");
    printf("\n");

    for (int i = 0; i < ast_sz; i++) {
        printf("stmt %d", i);

        VarList *in = IN[i], *out = OUT[i];

        while (in != NULL || out != NULL) {
            printf(" | ");

            int cursor = 8 + (int)log10(i+1) + 1;

            for (; in != NULL;) {
                int sym_str_sz = strlen(in->name) + log10(in->stmt_no + 1) + 4;
                if (sym_str_sz + cursor > COLUMN_LENGTH/2)
                    break;

                printf("%s(%d) ", in->name, in->stmt_no);

                cursor += sym_str_sz;
                in = in->next;
            }

            while (cursor++ < COLUMN_LENGTH/2 + 1)
                printf(" ");
            printf("| ");
            cursor++;

            for (; out != NULL;) {
                int sym_str_sz = strlen(out->name) + log10(out->stmt_no + 1) + 4;
                if (sym_str_sz + cursor > COLUMN_LENGTH)
                    break;

                printf("%s(%d) ", out->name, out->stmt_no);

                cursor += sym_str_sz;
                out = out->next;
            }
        }

        printf("\n");
        for (int j = 0; j < COLUMN_LENGTH; j++)
            printf("-");
        printf("\n");
    }
    printf("\n");
}
