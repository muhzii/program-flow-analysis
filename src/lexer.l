%option noyywrap

%{
    #include "value.h"
    #include "symtab.h"
    #include "parser.h"
%}


%%

[\n]  { yylineno++; }

"#include"([ ]+)?((<(\\.|[^>])+>)|(\"(\\.|[^"])+\"))  { return INCLUDE_STMT; }

"void"                       { return VOID_TYPE; }
"char"                       { return CHAR_TYPE; }
"short"|"int"|"long"         { return INTEGRAL_TYPE; }
"float"|"double"             { return FLOATING_TYPE; }

"if"    { return IF; }
"else"  { return ELSE; }

"for"  { return FOR; }

"while"  { return WHILE; }

"continue"  { return CONTINUE; }
"break"	    { return BREAK; }

"return"  { return RETURN; }

"||"|"&&"|"=="|"!="|"<="|">="  { return LOGICAL; }

"++"  { return INC; }
"--"  { return DEC; }

"="|","|"{"|"}"|"("|")"|"["|"]"|"+"|"-"|"/"|"|"|"^"|"%"|"<"|">"|"&"|"*"|"~"|"!"|";"  { return yytext[0]; }

"*="|"/="|"+="|"%="|">>="|"-="|"<<="|"&="|"^="|"|="  { return ASSIGN; }

[0-9]+  { yylval.i_value = atoi(yytext); return CONST_INT; }

[0-9]+"."[0-9]+  { yylval.f_value = atof(yytext); return CONST_FLOAT; }

"'"."'"  { yylval.c_value = yytext[0]; return CONST_CHAR; }

[a-zA-z_][a-zA-z_0-9]*  { symtab_insert(yytext, 0); yylval.sym = symtab_lookup(yytext); return ID; }

\"([^\\\"]|\\.)*\"  { return CONST_STRING; }

"//"(\\.|[^\n])*                /* Eat comment */;
[/][*]([^*]|[*]*[^*/])*[*]+[/]  /* Eat comment */;
[ \t]                           /* Eat whitespace */;

%%
