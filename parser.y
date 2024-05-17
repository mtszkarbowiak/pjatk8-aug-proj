%{

#include <stdio.h>
#include <stdlib.h>


#include "parser.tab.h"

int yylex(void);
void yyerror(const char *s);

%}

%token NUMBER IDENTIFIER PLUS MINUS MULTIPLY DIVIDE EOL

%%

expression:
    term
    | expression PLUS term
    | expression MINUS term
    ;

term:
    factor
    | term MULTIPLY factor
    | term DIVIDE factor
    ;

factor:
    NUMBER
    | IDENTIFIER
    ;

%%
