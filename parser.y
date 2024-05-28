%{

#include <stdio.h>
#include <stdlib.h>


#include "parser.tab.h"

#include "ast.h"


int yylex(void);
void yyerror(const char *s);

%}

%define parse.error detailed
%locations

%union {
	class AstNode* node;
	class StatementNode* statement;
	class ExpressionNode* expression;
}

%token STOP
%token STATEMENT_SEPARATOR BODY_OPEN BODY_CLOSE
%token LET ASSIGN OF_TYPE ASSERT
%token IF ELSE WHILE
%token TRUE FALSE NUMBER
%token IDENTIFIER
%token PLUS MINUS MULTIPLY DIVIDE MODULO 
%token EQUAL NOT_EQUAL LESS_THAN MORE_THAN LESS_EQUAL MORE_EQUAL
%token LOGIC_AND LOGIC_OR LOGIC_XOR
%token EOL

%start program

%left PLUS MINUS DIVIDE MULTIPLY MODULO
%left EQUAL NOT_EQUAL LESS_THAN MORE_THAN LESS_EQUAL MORE_EQUAL
%left LOGIC_AND LOGIC_OR LOGIC_XOR

%%

program:
	statements
	;

body:
	BODY_OPEN statements BODY_CLOSE
	| statement
	;

statements:
	statements STATEMENT_SEPARATOR statement
	| statement
	;


statement:
	expression
	| LET IDENTIFIER ASSIGN expression
	| LET IDENTIFIER OF_TYPE IDENTIFIER ASSIGN expression
	| IDENTIFIER ASSIGN expression 
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
	| expression EQUAL expression
	| expression NOT_EQUAL expression
	| expression LESS_THAN expression
	| expression MORE_THAN expression
	| expression LESS_EQUAL expression
	| expression MORE_EQUAL expression
	| expression LOGIC_AND expression
	| expression LOGIC_OR expression
	| expression LOGIC_XOR expression
	| TRUE
	| FALSE
	| NUMBER
	| IDENTIFIER
	;
%%
