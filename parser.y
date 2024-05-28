%{

#include <stdio.h>
#include <stdlib.h>


#include "parser.tab.h"

int yylex(void);
void yyerror(const char *s);

%}

%define parse.error detailed
%locations

%token STOP
%token LET EQUAL_TO OF_TYPE ASSERT
%token IF ELSE WHILE
%token TRUE FALSE NUMBER
%token IDENTIFIER
%token PLUS MINUS MULTIPLY DIVIDE MODULO 
%token EOL

%start program

%left PLUS MINUS
%left DIVIDE MULTIPLY MODULO

%%

program:
	statements
	;

body:
	'{' statements '}'
	;

statements:
	statements OF_TYPE statement
	| statement
	;


statement:
	expression
	| LET IDENTIFIER EQUAL_TO expression
	| LET IDENTIFIER OF_TYPE IDENTIFIER EQUAL_TO expression
	| IF expression body
	| IF expression body ELSE body
	| WHILE expression body
	| ASSERT expression
	;

expression:
	'(' expression ')'
	| expression MULTIPLY expression
	| expression DIVIDE expression
	| expression PLUS expression
	| expression MINUS expression
	| expression MODULO expression
	| TRUE
	| FALSE
	| NUMBER
	| IDENTIFIER
	;
%%
