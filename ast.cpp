#include "ast.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <valarray>


static void terminate_illegal_program(const std::string& reasoning) {
	std::cerr << "An error occured during execution. Reason:\n" + reasoning + "\nProgram terminated.";
	throw std::runtime_error("Illegal program can not be executed.");
}


Value::Value(Logic value) : value(value) {}
Value::Value(Number value) : value(value) {}
Value::Value(Text text) : value(text) {}


auto Variable::get_name() const -> const std::string&
{
	return this->name;
}

auto Variable::get_value() const -> const Value&
{
	return this->value;
}

auto Variable::get_value() -> Value&
{
	return this->value;
}


namespace ValueVisitors
{
	struct Printer final
	{
		std::stringbuf* target;

		void print_literal(const std::string& string) const {
			target->sputn(string.c_str(), string.length());
		}

		void operator()(Value::Logic) const {
			print_literal("Logic");
		}

		void operator()(Value::Number) const {
			print_literal("Number");
		}

		void operator()(Value::Text) const {
			print_literal("Text");
		}
	};


	struct ValueArithmeticVisitor final
	{

	};
}



// Creates a predicate to be used to search variable by its name.
auto get_var_name_predicate(std::string_view name)
{
	return [name](const Variable& var) -> bool {
		return var.get_name() == name;
	};
}


auto ExecutionScopedState::try_get_var_value(const std::string_view name) -> Value*
{
	auto result = std::find_if(
		variables.begin(),
		variables.end(),
		get_var_name_predicate(name)
	);

	const bool found = result != variables.end();

	return found ? (&result->get_value()) : nullptr;
}

auto ExecutionScopedState::try_get_var_value(const std::string_view name) const -> const Value*
{
	auto result = std::find_if(
		variables.begin(),
		variables.end(),
		get_var_name_predicate(name)
	);

	const bool found = result != variables.end();

	return found ? (&result->get_value()) : nullptr;
}

void ExecutionScopedState::declare_variable(Variable&& variable)
{
	const auto result = std::find_if(
		variables.begin(),
		variables.end(),
		get_var_name_predicate(variable.get_name())
	);

	if (result != variables.end()) {
		throw std::runtime_error("Variable");
	}

	this->variables.emplace_back(std::move(variable));
}



LiteralNode::LiteralNode(Value&& value)
	: ExpressionNode()
	, value(value)
{
}
UnaryExpressionNode::UnaryExpressionNode(
	const Operator op,
	ExpressionNode* child)
	: ExpressionNode()
	, operator_(op)
	, child(std::unique_ptr<ExpressionNode>(child))
{
}

BinaryExpressionNode::BinaryExpressionNode(
	const OperatorVariant op,
	ExpressionNode* left,
	ExpressionNode* right)
	: ExpressionNode()
	, operator_(op)
	, left_child(std::unique_ptr<ExpressionNode>(left))
	, right_child(std::unique_ptr<ExpressionNode>(right))
{
}

VariableReferenceNode::VariableReferenceNode(std::string&& name)
	: ExpressionNode()
	, name(std::move(name))
{
}



auto LiteralNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	return this->value;
}

auto UnaryExpressionNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	const Value child_value = this->child->evaluate(execution_scoped_state);

	Value result = child_value; //TODO Add the handler

	return result;
}

auto BinaryExpressionNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	const Value left_value = this->left_child->evaluate(execution_scoped_state);
	const Value right_value = this->right_child->evaluate(execution_scoped_state);

	Value result = left_value; //TODO Add the handler

	return result;
}

auto VariableReferenceNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	const Value* value = execution_scoped_state.try_get_var_value(this->name);

	if (value == nullptr) {
		terminate_illegal_program("Value is null and can not be evaluated.");
	}

	Value result = *value;
	return result;
}


void AstNode::print_padding(std::stringbuf& buf, const int32_t depth) const
{
	for (int i = 0; i < depth; ++i) {
		buf.sputn(" ", 1);
	}
}

void LiteralNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	this->value.handle(ValueVisitors::Printer{ &buf });
}

void UnaryExpressionNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	this->child->print(buf, depth + 1);
}

void BinaryExpressionNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	this->left_child->print(buf, depth + 1);
	this->right_child->print(buf, depth + 1);
}

void VariableReferenceNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	constexpr char ref_info[] = "Ref: ";
	buf.sputn(ref_info, std::size(ref_info));
	buf.sputn(name.c_str(), name.length());
}
