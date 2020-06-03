//
// Created by muhzi on 6/1/20.
//

#ifndef FLOW_ANALYSIS_SYMTAB_H
#define FLOW_ANALYSIS_SYMTAB_H

#include "value.h"

#include <stdio.h>

#define SYMTAB_SIZE 26


struct Symtab {
    char *s_name;
    int scope;

    int dirty;

    ValueRecord cur_vr;

    struct Symtab *next;
};

typedef struct Symtab Symtab;

Symtab **stab;
int curr_scope;

void symtab_init ();
void symtab_deinit ();
Symtab * symtab_lookup (char *name);
void symtab_insert (char *name, int define);
void symtab_update (char *name, ValueRecord vr);
Symtab * symtab_define (char *name);

// Debug.
void symtab_dump (FILE *dest);

void increment_scope ();
void decrement_scope ();

#endif //FLOW_ANALYSIS_SYMTAB_H
