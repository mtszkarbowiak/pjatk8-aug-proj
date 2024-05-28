
#include "interpreter.h"

#include <string>


auto lu() -> LexerUtil&
{
	static LexerUtil global_instance{};
	return global_instance;
}


int main()
{
    const auto parsing_result = yyparse();

	lu().print_diagnostics();

	return parsing_result;
}

void yyerror(const char* s)
{
	std::cout << "Error: " << s << '\n';
}



auto get_token_name(const int token) -> const char*
{
	switch (token)
	{
	case NUMBER:
		return "NUMBER";
	case IDENTIFIER:
		return "IDENTIFIER";
	default:
		return "???";
	}
}

auto LexerUtil::feed(const int token) -> int
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

