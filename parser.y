%{

#include <stdio.h>
#include <stdlib.h>

#include "parser.tab.h"
#include "ast.h"

int yylex(void);
void yyerror(const char *s);

class AstRoot* root;

%}

%define parse.error detailed
%locations

%union {
	int ival;
	bool bval;

	class Node* node;
	class StatementNode* statement_node;
	class ExpressionNode* expression_node;
}

%type <statement_node> statement
%type <statement_node> statements
%type <expression_node> expression


%token <bval> TRUE FALSE
%token <ival> NUMBER

%token STOP
%token STATEMENT_SEPARATOR BODY_OPEN BODY_CLOSE
%token LET ASSIGN OF_TYPE ASSERT
%token IF ELSE WHILE
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
	statements								{ root = new AstRoot($1); root->print_to_console(); root->execute(); }
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
	expression								{ $$ = new ResultNode($1); }
	| LET IDENTIFIER ASSIGN expression
	| LET IDENTIFIER OF_TYPE IDENTIFIER ASSIGN expression
	| IDENTIFIER ASSIGN expression 
	| IF expression body
	| IF expression body ELSE body
	| WHILE expression body
	| ASSERT expression
	;

expression:
	'(' expression ')'						{ $$ = new BraceExpressionNode($2); }

	| expression MULTIPLY expression		{ $$ = new BinaryOperationNode(ArithmeticOperation::Multiplication, $1, $3); }
	| expression DIVIDE expression			{ $$ = new BinaryOperationNode(ArithmeticOperation::Division, $1, $3); }
	| expression PLUS expression			{ $$ = new BinaryOperationNode(ArithmeticOperation::Addition, $1, $3); }
	| expression MINUS expression			{ $$ = new BinaryOperationNode(ArithmeticOperation::Substraction, $1, $3); }
	| expression MODULO expression			{ $$ = new BinaryOperationNode(ArithmeticOperation::Modulo, $1, $3); }

	| expression EQUAL expression			{ $$ = new BinaryOperationNode(ComparisonOperation::Equality, $1, $3); }
	| expression NOT_EQUAL expression		{ $$ = new BinaryOperationNode(ComparisonOperation::Inequality, $1, $3); }
	| expression LESS_THAN expression		{ $$ = new BinaryOperationNode(ComparisonOperation::Less, $1, $3); }
	| expression MORE_THAN expression		{ $$ = new BinaryOperationNode(ComparisonOperation::More, $1, $3); }
	| expression LESS_EQUAL expression		{ $$ = new BinaryOperationNode(ComparisonOperation::LessOrEqual, $1, $3); }
	| expression MORE_EQUAL expression		{ $$ = new BinaryOperationNode(ComparisonOperation::MoreOrEqual, $1, $3); }

	| expression LOGIC_AND expression		{ $$ = new BinaryOperationNode(LogicOperation::And, $1, $3); }
	| expression LOGIC_OR expression		{ $$ = new BinaryOperationNode(LogicOperation::Or, $1, $3); }
	| expression LOGIC_XOR expression		{ $$ = new BinaryOperationNode(LogicOperation::Xor, $1, $3); }

	| TRUE									{ $$ = new LiteralNode(Value($1)); }
	| FALSE									{ $$ = new LiteralNode(Value($1)); }
	| NUMBER								{ $$ = new LiteralNode(Value($1)); }

	| IDENTIFIER							{ $$ = new VariableReferenceNode("SomeReference"); /*TODO*/ }
	;
%%
