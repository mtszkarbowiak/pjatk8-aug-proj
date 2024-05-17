%{

#include <cstdio>
#include <iostream>

#include "parser.hpp"
#include "cesserract.h"

int yylex(void);
void yyerror(const char *s);

%}

%token NUMBER PLUS MINUS MUL DIV

%%

expression:
    expression PLUS expression
    | expression MINUS expression
    | expression MUL expression
    | expression DIV expression
    | NUMBER
    ;

%%

int main() {
    std::cout << "Hello CMake." << '\n';
    yyparse();
    return 0;
}
void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}
