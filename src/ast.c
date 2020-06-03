//
// Created by muhzi on 6/1/20.
//

#include "ast.h"

#include <stdlib.h>
#include <string.h>


static RefList * reflist_copy (RefList *rl);
static RefList * reflist_combine (RefList *l1, RefList *l2);
static void reflist_free (RefList *rl);

RefList *
reflist_new (Symtab *sym)
{
    RefList *l = malloc(sizeof(RefList));
    l->sym = sym;
    l->vr = sym->cur_vr;
    l->next = NULL;
    return l;
}

int
reflist_has_sym (RefList *rl, Symtab *sym)
{
    for (; rl != NULL; rl = rl->next)
        if (rl->sym == sym)
            return 1;

    return 0;
}

const char *
ast_type_to_str (NodeType type)
{
    return NODE_TYPE_STR[type];
}

AST *
ast_new (NodeType type, AST *next)
{
    AST *t = calloc(1, sizeof(AST));
    t->lineno = yylineno;
    t->type = type;
    t->next = next;
    t->scope = curr_scope;
    return t;
}

int
ast_get_size (AST *t)
{
    int size = 0;
    while (t != NULL) {
        size++;
        t = t->next;
    }
    return size;
}

AST *
ast_combine (AST *t1, AST *t2)
{
    AST *res = ast_new(t1->type, NULL);

    res->lineno = t1->lineno;

    res->reflist = reflist_combine(t1->reflist, t2->reflist);
    res->deflist = reflist_combine(t1->deflist, t2->deflist);

    ast_free(t1);
    ast_free(t2);

    return res;
}

void
ast_append (AST *t1, AST *t2)
{
    if (t1 == NULL)
        return;

    while (t1->next != NULL)
        t1 = t1->next;
    t1->next = t2;
}

void
ast_add_ref (AST *t, Symtab *sym)
{
    if (t == NULL)
        return;
    if (reflist_has_sym(t->reflist, sym))
        return;

    RefList *new_ref = reflist_new(sym);
    new_ref->next = t->reflist;
    t->reflist = new_ref;
}

void
ast_add_def (AST *t, Symtab *sym)
{
    if (t == NULL)
        return;
    if (reflist_has_sym(t->deflist, sym))
        return;

    RefList *new_def = reflist_new(sym);
    new_def->next = t->deflist;
    t->deflist = new_def;
}

void
ast_free (AST *t)
{
    if (t == NULL || t == &AST_NONE)
        return;

    reflist_free(t->reflist);
    reflist_free(t->deflist);

    ast_free(t->next);

    free(t);
}

AST *
build_while_stmt_ast (AST *expr, AST *stmt)
{
    AST *t = expr;
    t->type = NODE_TYPE_ITER_STMT;
    t->num_of_statements = ast_get_size(stmt);
    t->is_compound_stmt = stmt->is_compound_stmt;

    ast_append(t, stmt);

    return t;
}

AST *
build_for_stmt_ast (AST *expr1, AST *expr2, AST *expr3, AST *stmt)
{
    AST *t = ast_combine(expr1, expr2);
    t = ast_combine(t, expr3);
    t->type = NODE_TYPE_ITER_STMT;
    t->num_of_statements = ast_get_size(stmt);
    t->is_compound_stmt = stmt->is_compound_stmt;

    ast_append(t, stmt);

    return t;
}

AST *
build_if_stmt_ast (AST *expr, AST *stmt)
{
    AST *t = expr;
    t->type = NODE_TYPE_IF_STMT;
    t->num_of_statements = ast_get_size(stmt);
    t->is_compound_stmt = stmt->is_compound_stmt;

    ast_append(t, stmt);

    return t;
}

AST *
build_if_else_stmt_ast (AST *expr, AST *stmt, AST *else_stmt)
{
    AST *t = expr;
    t->type = NODE_TYPE_IF_STMT;
    t->num_of_statements = ast_get_size(stmt);
    t->is_compound_stmt = stmt->is_compound_stmt;

    ast_append(t, stmt);
    ast_append(t, else_stmt);

    else_stmt->type = NODE_TYPE_ELSE_STMT;
    else_stmt->num_of_statements = ast_get_size(else_stmt);
    else_stmt->is_compound_stmt = else_stmt->is_compound_stmt;

    return t;
}

AST *
build_assign_expr_ast (AST *lhs, AST *rhs)
{
    if (lhs == NULL || rhs == NULL)
        return rhs;

    if (lhs->reflist != NULL) {
        Symtab *def_sym = lhs->reflist->sym;

        // Promote reflist item.
        ast_add_def(rhs, def_sym);

        // Update symtab entry.
        symtab_update(def_sym->s_name, rhs->vr);
    }

    if (lhs != rhs)
        ast_free(lhs);

    return rhs;
 }

AST *
build_fndef_ast (RefList *params, AST* stmt)
{
    AST *t = ast_new(NODE_TYPE_FN_DEF, stmt);

    t->lineno = stmt->lineno - 1;
    t->deflist = params;

    return t;
}

AST *
build_fn_call_ast (AST *fn_expr, AST *params_expr_list)
{
    // Mark scanf calls as definitions.
    AST *t = params_expr_list;
    if (fn_expr->reflist != NULL && strcmp(fn_expr->reflist->sym->s_name, "scanf") == 0) {
        for (RefList *rlist = t->reflist; rlist != NULL; rlist = rlist->next)
            ast_add_def(t, rlist->sym);

        t->reflist = NULL;
    }
    return t;
}

AST *
build_postfix_expr_id_ast (Symtab *sym)
{
    AST *t = ast_new(NODE_TYPE_EXPR, NULL);

    ast_add_ref(t, sym);
    t->vr = sym->cur_vr;

    return t;
}

AST *
build_init_declaration_ast (Symtab *declarator, AST *initializer)
{
    // Update symtab entry.
    symtab_update(declarator->s_name, initializer->vr);

    // Recover from AST_NONE.
    AST *t = ast_combine(initializer, &AST_NONE);
    ast_add_def(t, declarator);

    return t;
}

static RefList *
reflist_copy (RefList *rl)
{
    RefList *res = NULL;
    for (; rl != NULL; rl = rl->next) {
        RefList *new_node = reflist_new(rl->sym);
        if (res == NULL) {
            res = new_node;
        } else {
            res->next = new_node;
            res = res->next;
        }
    }
    return res;
}

static RefList *
reflist_combine (RefList *l1, RefList *l2)
{
    RefList *res = reflist_copy(l1);

    if (res == NULL)
        return reflist_copy(l2);

    for (; l2 != NULL; l2 = l2->next) {
        if (reflist_has_sym(l1, l2->sym))
            continue;

        RefList *new_ref = reflist_new(l2->sym);
        new_ref->next = res;
        res = new_ref;
    }

    return res;
}

static void
reflist_free (RefList *rl)
{
    while (rl != NULL) {
        RefList *tmp = rl;
        rl = rl->next;
        free(tmp);
    }
}
