#pragma once


// CPP Includes
#include <iostream>
#include <vector>

// C Includes
#include <stdio.h>

// BISON Includes
// #include "parser.hpp"
#include "parser.tab.h"


/// <summary>
///	Returns the LexerUtil instance.
/// </summary>
auto lu() -> class LexerUtil&;


class LexerUtil final
{
	int32_t comment_level = 0;
	std::vector<std::string> log{};
	bool verbose_log = true;

public:
	/// <summary>
	///	Configures if full log is built during lexing.
	/// </summary>
	void set_verbose_log(bool v);


	/// <summary>
	///	Handles next token.
	/// </summary>
	auto feed(yytokentype token_type, const char* token_value = nullptr) -> int;

	/// <summary>
	///	Prints the log to std::cout.
	/// </summary>
	void print_log() const;


	auto get_comment_level() const -> int32_t;

	void increase_comment_level();

	void decrease_comment_level();
};

