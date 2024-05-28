
#include "lexing.h"

#include <any>
#include <iostream>


auto main() -> int
{
	lu().set_verbose_log(true);

	// Invoke Lexer and Parser
    const auto parsing_result = yyparse();

	if (parsing_result != 0) {
		lu().print_log();
	}
	else {
		std::cout << "Program finished";
	}

	return parsing_result;
}


auto lu() -> LexerUtil&
{
	static LexerUtil global_instance{};
	return global_instance;
}

void yyerror(const char* s)
{
	std::cout << "Error: " << s << '\n';
}



auto get_token_name(const yytokentype token) -> const char*
{
	switch (token)
	{
		case YYEMPTY:			return "YY Empty";
		case YYEOF:				return "YY EOF";
		case YYUNDEF:			return "YY Undef";
		case YYerror:			return "YY Error";

		case EOL:				return "End of Line";
		case STOP:				return "Program Termination Keyword";

		case LET:				return "Variable Assignment Keyword";
		case ASSERT:			return "Assert Operator";

		case ASSIGN:			return "Variable Assignment Operator";
		case OF_TYPE:			return "Type Declaration Operator";

		case IF:				return "If Control Flow Operator";
		case ELSE:				return "Else Control Flow Operator";
		case WHILE:				return "While Control Flow Operator";

		case TRUE:				return "Boolean Literal (True)";
		case FALSE:				return "Boolean Literal (False)";

		case NUMBER:			return "Number Literal";
		case IDENTIFIER:		return "Identifier";

		case PLUS:				return "Plus Arithmetic Operator";
		case MINUS:				return "Minus Arithmetic Operator";
		case MULTIPLY:			return "Multiply Arithmetic Operator";
		case DIVIDE:			return "Divide Arithmetic Operator";
		case MODULO:			return "Modulo Arithmetic Operator";

		case LOGIC_AND:			return "AND Logic Operator";
		case LOGIC_OR:			return "OR Logic Operator";
		case LOGIC_XOR:			return "XOR Logic Operator";

		case EQUAL:				return "Equality Operator";
		case NOT_EQUAL:			return "Inequality Operator";
		case LESS_THAN:			return "Less Than Operator";
		case MORE_THAN:			return "More Than Operator";
		case LESS_EQUAL:		return "Less Than Or Equal Operator";
		case MORE_EQUAL:		return "More Than Or Equal Operator";

		case STATEMENT_SEPARATOR: return "Statements Separator";
		case BODY_OPEN:			return "Body Opening Brace";
		case BODY_CLOSE:		return "Body Closing Brace";
		default:				return "???";
	}
}


void LexerUtil::set_verbose_log(const bool v)
{
	this->verbose_log = v;
}

auto LexerUtil::feed(const yytokentype token_type, const char* token_value) -> int
{
	if (verbose_log)
	{
		std::string msg = (token_value != nullptr)
			? (std::string(get_token_name(token_type)) + ": " + token_value)
			: (std::string(get_token_name(token_type)));

		log.emplace_back(std::move(msg));
	}

	return token_type;
}

void LexerUtil::print_log() const
{
	for (const auto& msg : log) {
		std::cout << msg << '\n';
	}
}


auto LexerUtil::get_comment_level() const -> int32_t
{
	return this->comment_level;
}

void LexerUtil::increase_comment_level()
{
	if (verbose_log) {
		std::string msg = "Comment level increased to " + std::to_string(this->comment_level + 1);
		log.emplace_back(std::move(msg));
	}

	++this->comment_level;
}

void LexerUtil::decrease_comment_level()
{
	if (this->comment_level <= 0) {
		std::string msg = "Comment level is already 0. Token ignored.";
		log.emplace_back(std::move(msg));
	}

	if (verbose_log) {
		std::string msg = "Comment level decreased to " + std::to_string(this->comment_level - 1);
		log.emplace_back(std::move(msg));
	}

	--this->comment_level;
}

