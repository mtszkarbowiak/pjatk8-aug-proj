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
	class Node* node;
	class StatementNode* statement_node;
	class ExpressionNode* expression_node;
}

%type <statement_node> statement
%type <expression_node> expression

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

	| expression MULTIPLY expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ArithmeticOperator::Multiplication, $1, $3); }
	| expression DIVIDE expression			{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ArithmeticOperator::Division, $1, $3); }
	| expression PLUS expression			{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ArithmeticOperator::Addition, $1, $3); }
	| expression MINUS expression			{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ArithmeticOperator::Substraction, $1, $3); }
	| expression MODULO expression			{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ArithmeticOperator::Modulo, $1, $3); }

	| expression EQUAL expression			{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ComparisonOperator::Equality, $1, $3); }
	| expression NOT_EQUAL expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ComparisonOperator::Inequality, $1, $3); }
	| expression LESS_THAN expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ComparisonOperator::Less, $1, $3); }
	| expression MORE_THAN expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ComparisonOperator::More, $1, $3); }
	| expression LESS_EQUAL expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ComparisonOperator::LessOrEqual, $1, $3); }
	| expression MORE_EQUAL expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::ComparisonOperator::MoreOrEqual, $1, $3); }

	| expression LOGIC_AND expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::LogicOperator::And, $1, $3); }
	| expression LOGIC_OR expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::LogicOperator::Or, $1, $3); }
	| expression LOGIC_XOR expression		{ $$ = new BinaryExpressionNode(BinaryExpressionNode::LogicOperator::Xor, $1, $3); }

	| TRUE									{ $$ = new LiteralNode(Value(true)); }
	| FALSE									{ $$ = new LiteralNode(Value(false)); }
	| NUMBER								{ $$ = new LiteralNode(Value(1)); /*TODO*/ }

	| IDENTIFIER							{ $$ = new VariableReferenceNode("SomeReference"); /*TODO*/ }
	;
%%
