%{
        #include "value.h"
        #include "ast.h"
        #include "analyzer.h"
        #include "symtab.h"

        int yylex (void);
        int yyerror (const char *s);

        extern int yylineno;

        static void begin_compound_stmt ();
        static void begin_scoped_stmt ();
        static void end_scoped_stmt (AST *stmt);

        static int is_scoped_stmt;
%}

%union { int i_value; double f_value; char c_value; struct Symtab *sym; struct RefList *deflist;
         struct AST *ast_node; struct ValueRecord value_record; }

%token INCLUDE_STMT FOR IF BREAK RETURN WHILE CONTINUE ASSIGN INC DEC CONST_STRING
%token VOID_TYPE INTEGRAL_TYPE FLOATING_TYPE CHAR_TYPE

%token <i_value> CONST_INT
%token <f_value> CONST_FLOAT
%token <c_value> CONST_CHAR
%token <sym> ID

%type <value_record> const_expr
%type <sym> direct_declarator declarator
%type <deflist> fn_params_decl fn_params_decl_list
%type <ast_node> fndef compound_stmt expr expr_no_commas initializer postfix_expr xexpr unary_exp
%type <ast_node> stmt_sequence stmt iteration_stmt if_stmt init_declaration_stmt for_init_stmt
%type <ast_node> init_declarator init_declarator_list return_stmt

%right ASSIGN '='
%left LOGICAL
%left '+' '-'
%left '*' '/' '%'

%nonassoc "then"
%nonassoc ELSE

%%
program: INCLUDE_STMT program
         | translation_unit
         ;

translation_unit: translation_unit definition
                  | definition
                  ;

definition: { begin_scoped_stmt(); } fndef
                        { analyze($2); end_scoped_stmt($2); ast_free($2); }
            | /* empty - TODO */
            ;

fndef: type_const declarator '(' fn_params_decl ')' compound_stmt
                { $$ = build_fndef_ast($4, $6); }
       ;

fn_params_decl: fn_params_decl_list
                | /* empty */
                        { $$ = NULL; }
                ;

fn_params_decl_list: type_const declarator
                                { $$ = reflist_new($2); }
                     | fn_params_decl_list ',' type_const declarator
                                { $$ = $1; $1->next = reflist_new($4); }
                     ;

declarator: pointer direct_declarator
                { $$ = $2; }
            | direct_declarator
            ;

direct_declarator: ID
                        { $$ = symtab_define($1->s_name); }
                   | ID '[' CONST_INT ']'
                        { $$ = symtab_define($1->s_name); }
                   ;

stmt_sequence: stmt_sequence stmt
                        { $$ = $1; ast_append($$, $2); }
               | stmt
               ;

stmt: init_declaration_stmt
      | { begin_compound_stmt(); } compound_stmt
                { $$ = $2; decrement_scope(); }
      | { begin_scoped_stmt(); } if_stmt
                { $$ = $2; end_scoped_stmt($2); }
      | { begin_scoped_stmt(); } iteration_stmt
                { $$ = $2; end_scoped_stmt($2); }
      | jump_stmt
                { $$ = ast_new(NODE_TYPE_JUMP_STMT, NULL); }
      | expr ';'
                { $$ = $1; $$->type = NODE_TYPE_EXP_STMT; }
      | return_stmt
      ;

init_declaration_stmt: type_const init_declarator_list ';'
                                { $$ = $2; $$->type = NODE_TYPE_INIT_DECL_STMT; }
                       ;

init_declarator_list: init_declarator
                      | init_declarator_list ',' init_declarator
                                { $$ = ast_combine($1, $3); }
                      ;

init_declarator	: declarator
                        { $$ = ast_new(NODE_TYPE_EXPR, NULL); }
                 | declarator '=' initializer
                        { $$ = build_init_declaration_ast($1, $3); }
                 ;

initializer: expr_no_commas
             | '{' initializer_list '}'
                        { $$ = &AST_NONE; }
             | '{' initializer_list ',' '}'
                        { $$ = &AST_NONE; }
             ;

initializer_list: initializer_list ',' initializer
                  | initializer
                  ;

compound_stmt: '{' stmt_sequence '}'
                        { $$ = $2; $$->is_compound_stmt = 1; };

iteration_stmt: WHILE '(' expr ')' stmt
                        { $$ = build_while_stmt_ast($3, $5);  }
                | FOR '(' for_init_stmt xexpr ';' xexpr ')' stmt
                        { $$ = build_for_stmt_ast($3, $4, $6, $8); }
                ;

for_init_stmt: ';'
                        { $$ = ast_new(NODE_TYPE_EXP_STMT, NULL); }
               | stmt
               ;

if_stmt: IF '(' expr ')' stmt %prec "then"
                { $$ = build_if_stmt_ast($3, $5); }
         | IF '(' expr ')' stmt ELSE stmt
                { $$ = build_if_else_stmt_ast($3, $5, $7); }
         ;

jump_stmt: CONTINUE ';'
           | BREAK ';'
           ;

return_stmt: RETURN ';'
                        { $$ = ast_new(NODE_TYPE_RETURN_STMT, NULL); }
             | RETURN expr_no_commas ';'
                        { $$ = $2; $$->type = NODE_TYPE_RETURN_STMT; }
             ;

xexpr: expr
       | /* empty */
                { $$ = ast_new(NODE_TYPE_EXPR, NULL); }
       ;

expr: expr_no_commas
      | expr ',' expr_no_commas
                { $$ = ast_combine($1, $3); }
      ;

expr_no_commas: '(' expr_no_commas ')'
                           { $$ = $2; }
                 | unary_exp
                 | expr_no_commas '=' expr_no_commas
                           { $$ = build_assign_expr_ast($1, $3); }
                 | expr_no_commas ASSIGN expr_no_commas
                           { /* TODO */ $$ = build_assign_expr_ast($1, $3); }
                 | expr_no_commas '+' expr_no_commas
                           { $$ = ast_combine($1, $3); $$->vr = value_record_add($1->vr, $3->vr); }
                 | expr_no_commas '-' expr_no_commas
                           { $$ = ast_combine($1, $3); $$->vr = value_record_sub($1->vr, $3->vr); }
                 | expr_no_commas '*' expr_no_commas
                           { $$ = ast_combine($1, $3); $$->vr = value_record_mul($1->vr, $3->vr); }
                 | expr_no_commas '/' expr_no_commas
                           { $$ = ast_combine($1, $3); $$->vr = value_record_dev($1->vr, $3->vr); }
                 | expr_no_commas '%' expr_no_commas
                           { $$ = ast_combine($1, $3); $$->vr = value_record_rem($1->vr, $3->vr); }
                 | expr_no_commas LOGICAL expr_no_commas
                           { $$ = ast_combine($1, $3); }
                 | const_expr
                           { $$ = ast_new(NODE_TYPE_EXPR, NULL); $$->vr = $1; }
                 | postfix_expr
                 ;

unary_exp: unary_op expr_no_commas
                { $$ = $2; }
           | INC postfix_expr
                { $2->vr = value_record_add($2->vr, UNIT_VALUE_RECORD);
                  $$ = build_assign_expr_ast($2, $2); }
           | DEC postfix_expr
                { $2->vr = value_record_sub($2->vr, UNIT_VALUE_RECORD);
                  $$ = build_assign_expr_ast($2, $2); }
           ;

const_expr: CONST_INT
                { $$.type = SYMVAL_TYPE_INTEGER; $$.value._i = $1; }
            | CONST_FLOAT
                { $$.type = SYMVAL_TYPE_FLOAT; $$.value._f = $1; }
            | CONST_CHAR
                { $$.type = SYMVAL_TYPE_CHAR; $$.value._c = $1; }
            | CONST_STRING
                { $$.type = SYMVAL_TYPE_UNKNOWN; }
            ;

postfix_expr: ID
                        { $$ = build_postfix_expr_id_ast($1); $1->dirty = 1; }
              | postfix_expr '[' CONST_INT ']'
              | postfix_expr '(' expr ')'
                        { $$ = build_fn_call_ast($1, $3); }
              | postfix_expr INC
                        { $1->vr = value_record_add($1->vr, UNIT_VALUE_RECORD);
                          $$ = build_assign_expr_ast($1, $1); }
              | postfix_expr DEC
                        { $1->vr = value_record_sub($1->vr, UNIT_VALUE_RECORD);
                          $$ = build_assign_expr_ast($1, $1); }
              ;

type_const: VOID_TYPE
            | INTEGRAL_TYPE
            | FLOATING_TYPE
            | CHAR_TYPE
            ;

pointer: '*'
         | pointer '*'
         ;

unary_op: '&'
          | '*'
          | '+'
          | '-'
          | '~'
          | '!'
          ;
%%

int
yyerror (const char* msg)
{
        fprintf(stderr, "Parser error.\nLine no. %d: %s.\n", yylineno, msg);
        return 1;
}

void
begin_compound_stmt ()
{
        if (!is_scoped_stmt)
                increment_scope();
        else
                is_scoped_stmt = 0;
}

void
begin_scoped_stmt ()
{
        increment_scope();
        is_scoped_stmt = 1;
}

void
end_scoped_stmt (AST *stmt)
{
        if (!stmt->is_compound_stmt)
                decrement_scope();
        if (is_scoped_stmt)
                is_scoped_stmt = 0;
}
