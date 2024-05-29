#include "ast.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <valarray>


void terminate_illegal_program(const std::string& reasoning) {
	std::cerr << "An error occured during execution. Reason:\n" + reasoning + "\nProgram terminated.";
	throw std::runtime_error("Illegal program can not be executed.");
}

template<typename T>
auto get_value_casted(const Value* input, const char* error_msg) -> T
{
	T result;

	if (const T* ptr = input->try_get<T>())
	{
		result = *ptr;
	}
	else
	{
		terminate_illegal_program(error_msg);
	}

	return result;
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
	struct ValuePrinter final
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

	struct BinaryOperation
	{
		const Value* left_value;
		const Value* right_value;
		Value* result;

		void operator()(ArithmeticOperation arithmetic_operation)
		{
			const Value::Number l = get_value_casted<Value::Number>(left_value, "Left operand must a number to execute arithmetic operation.");
			const Value::Number r = get_value_casted<Value::Number>(right_value, "Right operand must a number to execute arithmetic operation.");

			auto calc = [arithmetic_operation, l, r]() -> Value::Number
			{
				switch (arithmetic_operation) {
					case ArithmeticOperation::Addition:			return l + r;
					case ArithmeticOperation::Substraction:		return l - r;
					case ArithmeticOperation::Multiplication:	return l * r;
					case ArithmeticOperation::Division:			return l / r;
					case ArithmeticOperation::Modulo:			return l % r;
				}

				terminate_illegal_program("Unknown arithmetic operation.");
				return 0;
			};

			*result = Value(calc());
		}

		void operator()(LogicOperation logic_operation)
		{
			const Value::Logic l = get_value_casted<Value::Logic>(left_value, "Left operand must a boolean to execute arithmetic operation.");
			const Value::Logic r = get_value_casted<Value::Logic>(right_value, "Right operand must a boolean to execute arithmetic operation.");

			auto calc = [logic_operation, l, r]() -> Value::Number
			{
				switch (logic_operation) {
					case LogicOperation::And:	return l && r;
					case LogicOperation::Or:	return l || r;
					case LogicOperation::Xor:	return l ^ r;
				}

				terminate_illegal_program("Unknown logic operation.");
				return 0;
			};

			*result = Value(calc());
		}

		void operator()(ComparisonOperation comparison_operation)
		{
			terminate_illegal_program("Comparison operations are not yet supported.");
		}
	};
}



// Creates a predicate to be used to search variable by its name.
auto get_var_name_predicate(std::string_view name)
{
	return [name](const Variable& var) -> bool {
		return var.get_name() == name;
	};
}


ExecutionScopedState::ExecutionScopedState(ExecutionScopedState* parent_state)
	: parent_state(parent_state)
{
}

auto ExecutionScopedState::try_get_var_value(const std::string_view name) -> Value*
{
	auto result = std::find_if(
		variables.begin(),
		variables.end(),
		get_var_name_predicate(name)
	);

	const bool found = result != variables.end();

	if (!found && parent_state) {
		if (Value* parent_value = parent_state->try_get_var_value(name)) 
		{
			return parent_value;
		}
	}

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

	if (!found && parent_state) {
		if (const Value* parent_value = parent_state->try_get_var_value(name))
		{
			return parent_value;
		}
	}

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

void ExecutionScopedState::set_result(Value&& value)
{
	if (result.has_value()) {
		terminate_illegal_program("The algorithm has already declared returned value.");
	}

	this->result.emplace(std::move(value));
}

auto ExecutionScopedState::get_result() const -> const std::optional<Value>&
{
	return this->result;
}


LiteralNode::LiteralNode(Value&& value)
	: ExpressionNode()
	, value(value)
{
}
UnaryOperationNode::UnaryOperationNode(
	const Operator op,
	ExpressionNode* child)
	: ExpressionNode()
	, operator_(op)
	, child(std::unique_ptr<ExpressionNode>(child))
{
}

BinaryOperationNode::BinaryOperationNode(
	const OperationVariant op,
	ExpressionNode* left,
	ExpressionNode* right)
	: ExpressionNode()
	, operation_(op)
	, left_child(std::unique_ptr<ExpressionNode>(left))
	, right_child(std::unique_ptr<ExpressionNode>(right))
{
}

VariableReferenceNode::VariableReferenceNode(std::string&& name)
	: ExpressionNode()
	, name(std::move(name))
{
}

ResultNode::ResultNode(ExpressionNode* result_expression)
	: result_expression(std::unique_ptr<ExpressionNode>(result_expression))
{
}


auto LiteralNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	return this->value;
}

auto UnaryOperationNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	const Value child_value = this->child->evaluate(execution_scoped_state);

	Value result = child_value; //TODO Add the handler

	return result;
}

auto BinaryOperationNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	ValueVisitors::BinaryOperation visitor;

	Value result;
	const Value left_value = this->left_child->evaluate(execution_scoped_state);
	const Value right_value = this->right_child->evaluate(execution_scoped_state);

	visitor.result = &result;
	visitor.left_value = &left_value;
	visitor.right_value = &right_value;

	std::visit(visitor, this->operation_);

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



void ResultNode::execute(ExecutionScopedState& execution_scoped_state) const
{
	Value statement_result = this->result_expression->evaluate(execution_scoped_state);
	execution_scoped_state.set_result(std::move(statement_result));
}


void AstNode::print_padding(std::stringbuf& buf, const int32_t depth) const
{
	buf.sputn("\n", 1);
	for (int i = 0; i < depth; ++i) {
		buf.sputn(" ", 1);
	}
}

AstRoot::AstRoot(StatementNode* head_statement)
	: head_statement(head_statement)
{
}

void AstRoot::execute()
{
	ExecutionScopedState execution_state;
	this->head_statement->execute(execution_state);
	auto result = execution_state.get_result();

	if (result.has_value()) {
		std::stringbuf buf;
		ValueVisitors::ValuePrinter printer;
		printer.target = &buf;

		result.value().handle_by_visitor(printer);
		std::cout << "Executed with result: " << buf.str() << "\n";

	} else {
		std::cout << "Executed without result.\n";
	}
}

void AstRoot::print(std::stringbuf& buf, int32_t depth) const
{
	buf.sputn("Tree Root", 9);

	this->head_statement->print(buf, 0);
}

void AstRoot::print_to_console()
{
	std::stringbuf buffer{};
	this->print(buffer, 0);

	std::cout << buffer.str();
}

void LiteralNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	this->value.handle_by_visitor(ValueVisitors::ValuePrinter{ &buf });
}

void UnaryOperationNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	this->child->print(buf, depth + 1);
}

void BinaryOperationNode::print(std::stringbuf& buf, const int32_t depth) const
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

void ResultNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	constexpr char return_info[] = "Return";
	buf.sputn(return_info, std::size(return_info));

	this->result_expression->print(buf, depth + 1);
}
