//
// Created by muhzi on 6/1/20.
//

#include "symtab.h"
#include "parser.h"

#include <stdio.h>

#ifdef YYDEBUG
    int yydebug = 1;
#endif


static void print_usage (char *program);

int
main (int argc, char **argv)
{
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (freopen(argv[1], "r", stdin) == NULL) {
        fprintf(stderr, "error: file %s cannot be opened.", argv[1]);
        return 1;
    }

    symtab_init();

    int result = yyparse();

    //symtab_dump(stdout);
    symtab_deinit();

    return result;
}

void
print_usage (char *program)
{
    printf("%s <source_file>\n\n", program);
    printf("Run static analysis on a C source file.\n");
    printf("\nArguments:\n");
    printf("\tsource_file\t\tPath to target .c file.\n");
}
