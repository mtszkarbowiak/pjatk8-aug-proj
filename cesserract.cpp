#include <stdio.h>

#include "parser.tab.h"

int main() {
    return yyparse();
}
