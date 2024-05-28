#pragma once

#include <any>
#include <string>
#include <variant>
#include <vector>


using NumericalVar = int32_t;
using BooleanVar = bool;
using TextVar = std::string;


//TODO
// To wszystko jest do poprawy:
// 1. Zamiast przypinac dzieci, dzieci powinny byc przesuwane do unique'ow rodzicow.
// 2. W parserze ostatnie stworzone dziecko powinno wyladowac w unii.
// 3. Tworzenie rodzica nastapi przy pomocy unii.
// W ten sposob, unika sie dodatkowych struktur danych tj. stack.
// Dodatkowo jest on bezpieczniejszy bo wszystkie dzieci sa z unique'ami.


class Variable final
{
	std::string name;
	std::any value;

public:
	[[nodiscard]]
	auto get_name() const -> const std::string&;

	[[nodiscard]]
	auto get_value() const -> const std::any&;

	[[nodiscard]]
	auto get_value() -> std::any&;

	template<typename T>
	[[nodiscard]]
	auto is_type() const -> bool
	{
		return typeid(T) == value.type();
	}
};


class ExecutionContext final
{
	std::vector<Variable> variables;

public:
	auto try_get_var_value(std::string_view name) -> std::any*;

	void declare_variable(Variable&& variable);
};


/// Atomic element of AST.
class AstNode
{
	AstNode* parent;

protected:
	explicit AstNode(AstNode* parent);

	virtual void print_padding(std::stringbuf& buf, int32_t depth = 0) const;

public:
	virtual ~AstNode() = default;

	virtual void print(std::stringbuf& buf, int32_t depth = 0) const = 0;

	[[nodiscard]] auto get_parent() const -> AstNode*;
};


/// Node that can be executed.
class StatementNode : public AstNode
{
public:
	~StatementNode() override = default;
};


/// Node that can be evaluated.
class ExpressionNode : public AstNode
{
protected:
	void ensure_exists(const ExpressionNode*) const;

	void ensure_parent(const ExpressionNode&) const;

public:
	~ExpressionNode() override = default;
};

/// Atomic object of evaluation.
class LiteralNode final : public AstNode
{
	std::any value;
	ExpressionNode* expression;

	explicit LiteralNode(ExpressionNode* expression, std::any&& value);

public:
	static auto new_numerical(ExpressionNode* expression, NumericalVar value) -> LiteralNode*;
	static auto new_boolean(ExpressionNode* expression, BooleanVar value) -> LiteralNode*;

	auto print(std::stringbuf& buf, int32_t depth) const -> void override;

	~LiteralNode() override = default;
};


class UnaryExpressionNode : public ExpressionNode
{
public:
	enum class Operator
	{
		Minus,
		Not,
	};


private:
	Operator _operator;
	ExpressionNode* child;


public:
	void print(std::stringbuf& buf, int32_t depth) const override;

	/// Modifies child pointer. Note: Its parent must already be pointing to the parent (this).
	void attach(ExpressionNode& child);
};

class BinaryExpressionNode : public ExpressionNode
{
public:
	enum class ArithmeticOperator
	{
		Addition,
		Substraction,
		Multiplication,
		Divison,
		Modulo,
	};

	enum class LogicOperator
	{
		And,
		Or,
		Xor,
	};

	using OperatorVariant = std::variant<ArithmeticOperator, LogicOperator>;


private:
	OperatorVariant _operator;
	ExpressionNode* left_child;
	ExpressionNode* right_child;


public:
	void print(std::stringbuf& buf, int32_t depth) const override;

	/// Modifies left child pointer. Note: Its parent must already be pointing to the parent (this).
	void attach_left(ExpressionNode& left_child);

	/// Modifies right child pointer. Note: Its parent must already be pointing to the parent (this).
	void attach_right(ExpressionNode& right_child);
};
