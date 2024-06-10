#pragma once

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "ast.h"
#include "ast.h"
#include "ast.h"
#include "ast.h"
#include "ast.h"


class AstNode;
class StatementNode;
class ExpressionNode;


std::string str_to_cpp(const char* copy);


class Value final
{
public:
	using Logic = bool;
	using Number = int32_t;
	using Text = std::string;

private:
	std::variant<Logic, Number, Text> value;

public:
	explicit Value() = default;

	explicit Value(Logic value);
	explicit Value(Number value);
	explicit Value(Text text);

	~Value() = default;

	Value(const Value&) = default;
	Value(Value&&) noexcept = default;

	auto operator=(const Value&) -> Value& = default;
	auto operator=(Value&&) noexcept -> Value& = default;

	template<typename TVisitor>
	void handle_by_visitor(TVisitor visitor)
	{
		std::visit(visitor, this->value);
	}
	template<typename TVisitor>
	void handle_by_visitor(TVisitor visitor) const
	{
		std::visit(visitor, this->value);
	}

	template<typename T>
	auto try_get() -> T*
	{
		return std::get_if<T>(&value);
	}

	template<typename T>
	auto try_get() const -> const T*
	{
		return std::get_if<T>(&value);
	}


	void reassign(const Value& src);
};

class Variable final
{
	std::string name;
	Value value;


	Variable() = default;

public:
	explicit Variable(std::string name, Value init_value);

	[[nodiscard]]
	auto get_name() const -> const std::string&;

	[[nodiscard]]
	auto get_value() const -> const Value&;

	[[nodiscard]]
	auto get_value() -> Value&;
};

class Function;


enum class ArithmeticOperation
{
	Addition,
	Substraction,
	Multiplication,
	Division,
	Modulo,
};

enum class LogicOperation
{
	And,
	Or,
	Xor,
};

enum class ComparisonOperation
{
	Equality,
	Inequality,
	Less,
	LessOrEqual,
	More,
	MoreOrEqual,
};


class ExecutionScopedState final
{
	ExecutionScopedState* parent_state{};
	std::vector<Variable> variables;
	std::vector<Function> functions;
	std::optional<Value> result;
	int level = 0;

public:
	explicit ExecutionScopedState() = default;

	explicit ExecutionScopedState(ExecutionScopedState* parent_state);

	auto try_get_var_value(std::string_view name) -> Value*;

	auto try_get_var_value(std::string_view name) const -> const Value*;

	auto try_get_function(std::string_view name) const -> const Function*;

	void declare_variable(Variable&& variable);

	void declare_function(Function&& function);

	void set_result(Value&& value);

	auto get_result() const -> const std::optional<Value>&;

	void print_summary();
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

class AstRoot final : public AstNode
{
	std::unique_ptr<StatementNode> head_statement;

public:
	explicit AstRoot(StatementNode*);


	void execute();


	void print(std::stringbuf& buf, int32_t depth) const override;

	void print_to_console();
};


class ArgsListNode final : public AstNode
{
	std::string name;
	std::unique_ptr<ArgsListNode> next;

	void append_list(std::vector<std::string>&);

public:
	explicit ArgsListNode(std::string name);

	explicit ArgsListNode(std::string name, ArgsListNode* next);

	void print(std::stringbuf& buf, int32_t depth) const override;

	auto get_list() -> std::vector<std::string>;
};


class ExpressionNode : public AstNode
{
protected:
	void ensure_exists(const ExpressionNode*) const;

	void ensure_parent(const ExpressionNode&) const;

public:
	virtual auto evaluate(const ExecutionScopedState&) -> Value = 0;

	~ExpressionNode() override = default;
};

class BraceExpressionNode final : public ExpressionNode
{
	std::unique_ptr<ExpressionNode> braced_expression;


public:
	explicit BraceExpressionNode(ExpressionNode* braced_expression);

	void print(std::stringbuf& buf, int32_t depth) const override;

	auto evaluate(const ExecutionScopedState&) -> Value override;
};

class LiteralNode final : public ExpressionNode
{
	Value value;
	
	
public:
	explicit LiteralNode(Value&& value);

	auto evaluate(const ExecutionScopedState&) -> Value override;

	auto print(std::stringbuf& buf, int32_t depth) const -> void override;

	~LiteralNode() override = default;
};

class UnaryOperationNode final : public ExpressionNode
{
public:
	enum class Operator { Minus, Not };

	explicit UnaryOperationNode(Operator op, ExpressionNode* child);

	auto evaluate(const ExecutionScopedState&) -> Value override;

	void print(std::stringbuf& buf, int32_t depth) const override;


private:
	Operator operator_;
	std::unique_ptr<ExpressionNode> child;
};

class BinaryOperationNode final : public ExpressionNode
{
public:
	using OperationVariant = std::variant<
		ArithmeticOperation,
		LogicOperation,
		ComparisonOperation>;

	explicit BinaryOperationNode(
		OperationVariant op,
		ExpressionNode* left,
		ExpressionNode* right
	);

	auto evaluate(const ExecutionScopedState&) -> Value override;

	void print(std::stringbuf& buf, int32_t depth) const override;

private:
	OperationVariant operation_;
	std::unique_ptr<ExpressionNode> left_child;
	std::unique_ptr<ExpressionNode> right_child;
};

class VariableReferenceNode final : public ExpressionNode
{
	std::string name;

public:
	explicit VariableReferenceNode(std::string&& name);

	void print(std::stringbuf& buf, int32_t depth) const override;

	auto evaluate(const ExecutionScopedState&) -> Value override;
};


class StatementNode : public AstNode
{
public:
	virtual void execute(ExecutionScopedState&) const = 0;

	~StatementNode() override = default;
};

class MultiStatementsNode final : public StatementNode
{
	std::unique_ptr<StatementNode> left_statement;
	std::unique_ptr<StatementNode> right_statement;

	MultiStatementsNode() = default;

public:
	explicit MultiStatementsNode(StatementNode* left_statement, StatementNode* right_statement);

	void print(std::stringbuf& buf, int32_t depth) const override;

	void execute(ExecutionScopedState&) const override;
};

class BodyNode final : public StatementNode
{
	std::unique_ptr<StatementNode> body_statement;

	BodyNode() = default;

public:
	explicit BodyNode(StatementNode* body_statement);

	void print(std::stringbuf& buf, int32_t depth) const override;

	void execute(ExecutionScopedState&) const override;
};

class ResultNode final : public StatementNode
{
	std::unique_ptr<ExpressionNode> result_expression;

	ResultNode() = default;

public:
	explicit ResultNode(ExpressionNode* result_expression);

	void execute(ExecutionScopedState&) const override;

	void print(std::stringbuf& buf, int32_t depth) const override;

	~ResultNode() override = default;
};

class VariableAssignmentNode final : public StatementNode
{
	std::string variable_name;
	std::unique_ptr<ExpressionNode> expression;
	bool is_reassignment;

public:
	explicit VariableAssignmentNode(std::string variable_name, ExpressionNode* expression, bool reassignment);

	void print(std::stringbuf& buf, int32_t depth) const override;

	void execute(ExecutionScopedState& context) const override;
};

class ConditionalStatementNode final : public StatementNode
{
	std::unique_ptr<ExpressionNode> condition;
	std::unique_ptr<StatementNode> statement;
	bool repeating;

	ConditionalStatementNode() = default;

public:
	explicit ConditionalStatementNode(
		ExpressionNode* condition, 
		StatementNode* statement,
		bool repeating);

	void print(std::stringbuf& buf, int32_t depth) const override;

	void execute(ExecutionScopedState&) const override;
};

class Function final
{
	std::string name;
	StatementNode* body;
	std::vector<std::string> signature;

	Function() = default;

public:
	using Signature = std::vector<std::string>;

	explicit Function(std::string name, StatementNode* body, Signature signature);

	auto call(const ExecutionScopedState&, const std::vector<std::string>& args) const -> std::optional<Value>;

	auto get_name() const -> const std::string&;
};



class FunctionDeclarationNode final : public StatementNode
{
	std::string name;
	std::unique_ptr<StatementNode> body;
	std::unique_ptr<ArgsListNode> args;

	FunctionDeclarationNode() = default;

public:
	explicit FunctionDeclarationNode(
		std::string name, 
		StatementNode* body_node,
		ArgsListNode* args
	);

	void print(std::stringbuf& buf, int32_t depth) const override;

	void execute(ExecutionScopedState&) const override;
};

class FunctionCallNode final : public ExpressionNode, public StatementNode
{
	std::string name;
	std::unique_ptr<ArgsListNode> args;

	FunctionCallNode() = default;

	auto call(const ExecutionScopedState&) const -> std::optional<Value>;

public:
	explicit FunctionCallNode(
		std::string name,
		ArgsListNode* args
	);


	void print(std::stringbuf& buf, int32_t depth) const override;

	auto evaluate(const ExecutionScopedState&) -> Value override;

	void execute(ExecutionScopedState&) const override;
};