#pragma once

#include <any>
#include <memory>
#include <string>
#include <variant>
#include <vector>


using NumericalVar = int32_t;
using BooleanVar = bool;
using TextVar = std::string;


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


class ExecutionScopedState final
{
	std::vector<Variable> variables;

public:
	auto try_get_var_value(std::string_view name) -> std::any*;

	void declare_variable(Variable&& variable);
};


// --- Note ---
// Bison (for C) is not compatible with move-only types such as unique_ptr.
// When the union uses move-only types, it fails to be copied in Bison's internals.
// To mitigate this issue, the union uses raw pointer. Of course, this solution
// is regarded unsafe in C++ world. Therefor all nodes take raw pointers to children
// as constructor parameters. Then such parent takes ownership by wrapping the
// incoming chin in unique_ptr. A child must never be passed to multiple parents.
// Such operation does not makes sense conceptually and can corrupt memory.


class AstNode
{
protected:
	virtual void print_padding(std::stringbuf& buf, int32_t depth = 0) const;

public:
	virtual ~AstNode() = default;

	virtual void print(std::stringbuf& buf, int32_t depth = 0) const = 0;
};


class ExpressionNode : public AstNode
{
protected:
	void ensure_exists(const ExpressionNode*) const;

	void ensure_parent(const ExpressionNode&) const;

public:
	~ExpressionNode() override = default;
};

class LiteralNode final : public ExpressionNode
{
	std::any value;
	
public:
	explicit LiteralNode(NumericalVar numerical_value);
	explicit LiteralNode(BooleanVar boolean_value);

	auto print(std::stringbuf& buf, int32_t depth) const -> void override;

	// virtual auto evaluate() -> std::any = 0; //TODO

	~LiteralNode() override = default;
};


class UnaryExpressionNode final : public ExpressionNode
{
public:
	enum class Operator { Minus, Not };

	explicit UnaryExpressionNode(Operator op, ExpressionNode* child);

	void print(std::stringbuf& buf, int32_t depth) const override;


private:
	Operator operator_;
	std::unique_ptr<ExpressionNode> child;
};

class BinaryExpressionNode final : public ExpressionNode
{
public:
	enum class ArithmeticOperator
	{
		Addition,
		Substraction,
		Multiplication,
		Division,
		Modulo,
	};

	enum class LogicOperator
	{
		And,
		Or,
		Xor,
	};

	enum class ComparisonOperator
	{
		Equality,
		Inequality,
		Less,
		LessOrEqual,
		More,
		MoreOrEqual,
	};

	using OperatorVariant = std::variant<
		ArithmeticOperator,
		LogicOperator,
		ComparisonOperator>;


	explicit BinaryExpressionNode(
		OperatorVariant op,
		ExpressionNode* left,
		ExpressionNode* right
	);

	void print(std::stringbuf& buf, int32_t depth) const override;


private:
	OperatorVariant operator_;
	std::unique_ptr<ExpressionNode> left_child;
	std::unique_ptr<ExpressionNode> right_child;
};


class VariableReferenceNode final : public ExpressionNode
{
	std::string name;

public:
	explicit VariableReferenceNode(std::string&& name);

	void print(std::stringbuf& buf, int32_t depth) const override;
};

class StatementNode : public AstNode
{
public:
	explicit StatementNode();

	// virtual void execute(ExecutionContext& context) const = 0; //TODO

	~StatementNode() override = default;
};
