#pragma once

#include <any>
#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "ast.h"
#include "ast.h"


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



using NodePtr = std::unique_ptr<class AstNode>;
using ExpressionNodePtr = std::unique_ptr<class ExpressionNode>;
using StatementNodePtr = std::unique_ptr<class StatementNode>;


/// Atomic element of AST.
class AstNode
{
protected:
	virtual void print_padding(std::stringbuf& buf, int32_t depth = 0) const;

public:
	virtual ~AstNode() = default;

	virtual void print(std::stringbuf& buf, int32_t depth = 0) const = 0;
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
	ExpressionNodePtr child;
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
	ExpressionNodePtr left_child;
	ExpressionNodePtr right_child;
};


class VariableReferenceNode final : public ExpressionNode
{
	std::string name;

public:
	explicit VariableReferenceNode(std::string&& name);

	void print(std::stringbuf& buf, int32_t depth) const override;
};



/// Node that can be executed.
class StatementNode : public AstNode
{
public:
	explicit StatementNode();

	// virtual void execute(ExecutionContext& context) const = 0; //TODO

	~StatementNode() override = default;
};
