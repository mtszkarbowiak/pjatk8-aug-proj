#pragma once


// CPP Includes
#include <any>
#include <iostream>
#include <map>
#include <vector>

// C Includes
#include <stdio.h>

// BISON Includes
// #include "parser.hpp"

#include "parser.tab.h"


/// <summary>
///	Returns the LexerUtil instance.
/// </summary>
[[nodiscard]] auto lu() -> class LexerUtil&;


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


	[[nodiscard]] auto get_comment_level() const -> int32_t;

	void increase_comment_level();

	void decrease_comment_level();
};

class Variable final
{
	std::any value;

public:
	template<typename T>
	[[nodiscard]] auto is_type() const -> bool
	{
		return typeid(T) == value.type();
	}
};

class Execution final
{
	Variable result;
};

class Ast final
{
	class LiteralNode;
	class UnaryExpressionNode;
	class BinaryExpressionNode;
	class VariableDeclarationNode;
	class VariableReassignmentNode;
};
