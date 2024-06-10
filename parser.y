%{

#include <stdio.h>
#include <stdlib.h>

#include <string>

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
	char* str;

	class Node* node;
	class StatementNode* statement_node;
	class ExpressionNode* expression_node;
}

%type <statement_node> statement
%type <statement_node> statements
%type <statement_node> body
%type <expression_node> expression


%token <bval> TRUE FALSE
%token <ival> NUMBER
%token <str> IDENTIFIER

%token STOP
%token STATEMENT_SEPARATOR BODY_OPEN BODY_CLOSE
%token LET ASSIGN OF_TYPE ASSERT
%token IF ELSE WHILE
%token FUNC
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
	statements								{ root = new AstRoot($1); /* root->print_to_console(); */ root->execute(); }
	;

body:
	BODY_OPEN statements BODY_CLOSE			{ $$ = new BodyNode($2); }
	;

statements:
	statement STATEMENT_SEPARATOR statements { $$ = new MultiStatementsNode($1, $3); }
	| statement STATEMENT_SEPARATOR			{ $$ = $1; }
	;


statement:
	IDENTIFIER '(' ')'						{ $$ = new FunctionCallNode($1); }
	| LET IDENTIFIER ASSIGN expression		{ $$ = new VariableAssignmentNode(str_to_cpp($2), $4, false); }
	| IDENTIFIER ASSIGN expression			{ $$ = new VariableAssignmentNode(str_to_cpp($1), $3, true); }
	| IF expression body					{ $$ = new ConditionalStatementNode($2, $3, false); }
	| WHILE expression body					{ $$ = new ConditionalStatementNode($2, $3, true); }
	| FUNC IDENTIFIER '(' ')' body			{ $$ = new FunctionDeclarationNode($2, $5); }
	| expression							{ $$ = new ResultNode($1); }
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

	| IDENTIFIER '(' ')'					{ $$ = new FunctionCallNode($1); }
	| IDENTIFIER							{ $$ = new VariableReferenceNode($1); }
	;
%%
