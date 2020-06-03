//
// Created by muhzi on 6/1/20.
//

#ifndef FLOW_ANALYSIS_ANALYZER_H
#define FLOW_ANALYSIS_ANALYZER_H

#include "ast.h"


struct VarList {
    int stmt_no;
    char *name;
    int scope;

    struct VarList *next;
    struct VarList *prev;
};

typedef struct VarList VarList;

// Called by the parser for each function definition.
void analyze (AST *t);

void process_reaching_definitions (AST *t);
void process_propagated_constants (AST *t);

#endif //FLOW_ANALYSIS_ANALYZER_H
