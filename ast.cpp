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



AstNode::AstNode(AstNode* parent)
	: parent(parent)
{
	if (!parent) {
		throw std::runtime_error("Every node must have a parent.");
	}
}

void AstNode::print_padding(std::stringbuf& buf, const int32_t depth) const
{
	for (int i = 0; i < depth; ++i) {
		buf.sputn(" ", 1);
	}
}

auto AstNode::get_parent() const -> AstNode*
{
	return parent;
}

void ExpressionNode::ensure_exists(const ExpressionNode* potential_child) const
{
	if (potential_child == nullptr) {
		throw std::runtime_error("The child does not exist.");
	}
}

void ExpressionNode::ensure_parent(const ExpressionNode& potential_child) const
{
	if (potential_child.get_parent() != this) {
		throw std::runtime_error("Attaching a node requires its child to already know its parent.");
	}
}


LiteralNode::LiteralNode(ExpressionNode* expression, std::any&& value)
	: AstNode(expression)
	, value(std::move(value))
	, expression(expression)
{
	if (!this->value.has_value()) {
		throw std::runtime_error("Literal must have a value.");
	}
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

	ensure_exists(this->child);

	this->child->print(buf, depth + 1);
}

void BinaryExpressionNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	ensure_exists(this->left_child);
	ensure_exists(this->right_child);

	this->left_child->print(buf, depth + 1);
	this->right_child->print(buf, depth + 1);
}


void UnaryExpressionNode::attach(ExpressionNode& child)
{
	ensure_parent(child);

	this->child = &child;
}

void BinaryExpressionNode::attach_left(ExpressionNode& left_child)
{
	ensure_parent(left_child);

	this->left_child = &left_child;
}

void BinaryExpressionNode::attach_right(ExpressionNode& right_child)
{
	ensure_parent(right_child);

	this->right_child = &right_child;
}
