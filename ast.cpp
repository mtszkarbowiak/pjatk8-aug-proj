#include "ast.h"

#include <sstream>
#include <stdexcept>


auto Variable::get_name() const -> const std::string&
{
	return this->name;
}

auto Variable::get_value() const -> const std::any&
{
	return this->value;
}

auto Variable::get_value() -> std::any&
{
	return this->value;
}


// Creates a predicate to be used to search variable by its name.
auto get_var_name_predicate(std::string_view name)
{
	return [name](const Variable& var) -> bool {
		return var.get_name() == name;
	};
}


auto ExecutionContext::try_get_var_value(const std::string_view name) -> std::any*
{
	auto result = std::find_if(
		variables.begin(),
		variables.end(),
		get_var_name_predicate(name)
	);

	const bool found = result != variables.end();

	return found ? (&result->get_value()) : nullptr;
}

void ExecutionContext::declare_variable(Variable&& variable)
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




void AstNode::print_padding(std::stringbuf& buf, const int32_t depth) const
{
	for (int i = 0; i < depth; ++i) {
		buf.sputn(" ", 1);
	}
}



LiteralNode::LiteralNode(NumericalVar numerical_value)
	: ExpressionNode()
	, value(numerical_value)
{
}

LiteralNode::LiteralNode(BooleanVar boolean_value)
	: ExpressionNode()
	, value(boolean_value)
{
}

UnaryExpressionNode::UnaryExpressionNode(
	const Operator op,
	ExpressionNode* child)
	: ExpressionNode()
	, operator_(op)
	, child(ExpressionNodePtr(child))
{
}

BinaryExpressionNode::BinaryExpressionNode(
	const OperatorVariant op,
	ExpressionNode* left,
	ExpressionNode* right)
	: ExpressionNode()
	, operator_(op)
	, left_child(ExpressionNodePtr(left))
	, right_child(ExpressionNodePtr(right))
{
}



void LiteralNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	if (this->value.has_value())
	{
		const std::string name = this->value.type().name();
		buf.sputn(name.c_str(), name.length());
	}
	else
	{
		constexpr char null_info[] = "<null>";
		buf.sputn(null_info, std::size(null_info));
	}
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
