//
// Created by muhzi on 6/1/20.
//

#include "symtab.h"

#include <stdlib.h>
#include <string.h>


void
symtab_init ()
{
    stab = malloc(SYMTAB_SIZE * sizeof(Symtab *));
    for (int i = 0; i < SYMTAB_SIZE; i++)
        stab[i] = NULL;
}

void
symtab_deinit ()
{
    for (int i = 0; i < SYMTAB_SIZE; i++) {
        Symtab *sym = stab[i];
        while (sym != NULL) {
            free(sym->s_name);

            Symtab *tmp = sym;
            sym = sym->next;
            free(tmp);
        }
    }
}

unsigned int
symtab_hash (char *key)
{
    unsigned int result = 0;
    for (; *key != '\0'; key++)
        result += *key;

    return result % SYMTAB_SIZE;
}

Symtab *
symtab_lookup (char *name)
{
    Symtab *sym = NULL, *sym_list = stab[symtab_hash(name)];
    for (; sym_list != NULL; sym_list = sym_list->next) {
        if (strcmp(name, sym_list->s_name) == 0 && curr_scope >= sym_list->scope) {
            if (sym == NULL || sym_list->scope > sym->scope)
                sym = sym_list;
        }
    }
    return sym;
}

void
symtab_insert (char *name, int rescope)
{
    Symtab *sym = symtab_lookup(name);
    if (sym != NULL && !rescope)
        return;

    Symtab *new_sym = malloc(sizeof(Symtab));
    new_sym->s_name = strdup(name);
    new_sym->scope = curr_scope;
    new_sym->cur_vr = NO_VALUE_RECORD;
    new_sym->dirty = 0;

    unsigned int hash_value = symtab_hash(name);
    new_sym->next = stab[hash_value];
    stab[hash_value] = new_sym;
}

Symtab *
symtab_define (char *name)
{
    Symtab *sym = symtab_lookup(name);
    if (sym != NULL && sym->dirty && curr_scope == sym->scope) {
        fprintf(stderr, "Symtab error: Redefinition of variable %s.\n", name);
        exit(1);
    } else if (sym != NULL && !sym->dirty && curr_scope == sym->scope) {
        sym->dirty = 1;
        return sym;
    }

    symtab_insert(name, 1);

    sym = symtab_lookup(name);
    sym->dirty = 1;

    return sym;
}

void
symtab_update (char *name, ValueRecord vr)
{
    Symtab *sym = symtab_lookup(name);
    if (sym == NULL) {
        fprintf(stderr, "Symtab error: undeclared variable %s.\n", name);
        exit(1);
    }

    sym->cur_vr = vr;
}

void
symtab_dump (FILE *dest)
{
    printf("*** Symbol table entries ***\n");
    printf("no.\tName\tScope\tValue\n\n");

    int cnt = 0;
    for (int i = 0; i < SYMTAB_SIZE; i++) {
        Symtab *sym = stab[i];
        if (sym == NULL)
            continue;

        for (; sym != NULL; sym = sym->next, cnt++) {
            switch (sym->cur_vr.type) {
                case SYMVAL_TYPE_INTEGER:
                    printf("%d:\t%s\t%d\t%d\n", cnt,
                            sym->s_name, sym->scope, sym->cur_vr.value._i);
                    break;
                case SYMVAL_TYPE_FLOAT:
                    printf("%d:\t%s\t%d\t%f\n", cnt,
                            sym->s_name, sym->scope, sym->cur_vr.value._f);
                    break;
                case SYMVAL_TYPE_CHAR:
                    printf("%d:\t%s\t%d\t%c\n", cnt,
                            sym->s_name, sym->scope, sym->cur_vr.value._c);
                    break;
                case SYMVAL_TYPE_UNKNOWN:
                    printf("%d: \t%s\t%d\n", cnt, sym->s_name, sym->scope);
                    break;
            }
        }
    }
}

void
increment_scope ()
{
    curr_scope++;
}

void
decrement_scope ()
{
    curr_scope--;
}
