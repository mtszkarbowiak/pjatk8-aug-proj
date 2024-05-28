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


class LexerUtil
{
	int32_t comment_level = 0;
	std::vector<std::string> diagnostics{};
	bool verbose = true;

public:
	void set_verbose(bool verbose);

	auto feed(yytokentype token_type, const char* token_value = nullptr) -> int;
	void print_diagnostics() const;

	auto get_comment_level() const -> int32_t;
	void increase_comment_level();
	void decrease_comment_level();
};

