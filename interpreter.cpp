
#include "interpreter.h"

#include <string>


auto main() -> int
{
	lu().set_verbose(true);

	// Invoke Lexer and Parser
    const auto parsing_result = yyparse();

	if (parsing_result != 0) 
	{
		lu().print_diagnostics();
	}
	else
	{
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

		case EQUAL_TO:			return "Variable Assignment Operator";
		case OF_TYPE:			return "Type Declaration Operator";

		case IF:				return "If Control Flow Operator";
		case ELSE:				return "Else Control Flow Operator";
		case WHILE:				return "While Control Flow Operator";

		case TRUE:				return "Boolean Literal (True)";
		case FALSE:				return "Boolean Literal (False)";

		case NUMBER:			return "Number Literal";
		case IDENTIFIER:		return "Identifier";

		case PLUS:				return "Plus";
		case MINUS:				return "Minus";
		case MULTIPLY:			return "Multiply";
		case DIVIDE:			return "Divide";
		case MODULO:			return "Modulo";

		default:				return "???";
	}
}


void LexerUtil::set_verbose(const bool verbose)
{
	this->verbose = verbose;
}

auto LexerUtil::feed(const yytokentype token) -> int
{
	if (verbose)
	{
		std::string msg = "Token: " + std::string(get_token_name(token));
		diagnostics.emplace_back(std::move(msg));
	}

	return token;
}

void LexerUtil::print_diagnostics() const
{
	for (const auto& msg : diagnostics)
	{
		std::cout << msg << '\n';
	}
}


auto LexerUtil::get_comment_level() const -> int32_t
{
	return this->comment_level;
}

void LexerUtil::increase_comment_level()
{
	if (verbose) 
	{
		std::string msg = "Comment level increased to " + std::to_string(this->comment_level + 1);
		diagnostics.emplace_back(std::move(msg));
	}

	++this->comment_level;
}

void LexerUtil::decrease_comment_level()
{
	if (this->comment_level <= 0) 
	{
		std::string msg = "Comment level is already 0. Token ignored.";
		diagnostics.emplace_back(std::move(msg));
	}

	if (verbose) 
	{
		std::string msg = "Comment level decreased to " + std::to_string(this->comment_level - 1);
		diagnostics.emplace_back(std::move(msg));
	}

	--this->comment_level;
}

