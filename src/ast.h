//
// Created by muhzi on 6/1/20.
//

#ifndef FLOW_ANALYSIS_AST_H
#define FLOW_ANALYSIS_AST_H

#include "symtab.h"


struct RefList {
    Symtab *sym;
    ValueRecord vr;

    struct RefList *next;
};

#define FOREACH_NODE_TYPE(FN) \
    FN(NODE_TYPE_FN_DEF) \
    FN(NODE_TYPE_IF_STMT) \
    FN(NODE_TYPE_ELSE_STMT) \
    FN(NODE_TYPE_ITER_STMT) \
    FN(NODE_TYPE_INIT_DECL_STMT) \
    FN(NODE_TYPE_EXP_STMT) \
    FN(NODE_TYPE_EXPR) \
    FN(NODE_TYPE_JUMP_STMT) \
    FN(NODE_TYPE_RETURN_STMT)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_ENUM_STR(STR) #STR,

enum NodeType {
    FOREACH_NODE_TYPE(GENERATE_ENUM)
};

static const char *NODE_TYPE_STR[] = {
    FOREACH_NODE_TYPE(GENERATE_ENUM_STR)
};

struct AST {
    int lineno;
    enum NodeType type;

    int scope;

    // For compound statements.
    int is_compound_stmt;
    int num_of_statements;

    ValueRecord vr;

    struct RefList *deflist;
    struct RefList *reflist;

    struct AST *next;
};

typedef struct RefList RefList;
typedef struct AST AST;
typedef enum NodeType NodeType;

extern int yylineno;

AST AST_NONE;

RefList * reflist_new (Symtab *sym);
int reflist_has_sym (RefList *rl, Symtab *sym);

const char * ast_type_to_str (NodeType type);

AST * ast_new (NodeType type, AST *next);
int ast_get_size (AST *t);
AST * ast_combine (AST *t1, AST *t2);
void ast_append (AST *t1, AST *t2);
void ast_add_ref (AST *t, Symtab *sym);
void ast_add_def (AST *t, Symtab *sym);
void ast_free (AST *t);

AST * build_while_stmt_ast (AST *expr, AST *stmt);
AST * build_for_stmt_ast (AST *expr1, AST *expr2, AST *expr3, AST *stmt);
AST * build_if_stmt_ast (AST *expr, AST *stmt);
AST * build_if_else_stmt_ast (AST *expr, AST *stmt, AST *else_stmt);
AST * build_assign_expr_ast (AST *lhs, AST *rhs);
AST * build_fndef_ast (RefList *params, AST* stmt);
AST * build_fn_call_ast (AST *fn_expr, AST *params_expr_list);
AST * build_postfix_expr_id_ast (Symtab *sym);
AST * build_init_declaration_ast (Symtab *declarator, AST *initializer);

#endif //FLOW_ANALYSIS_AST_H
