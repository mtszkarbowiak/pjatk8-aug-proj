#include "ast.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <valarray>


[[noreturn]]
void terminate_illegal_program(const std::string& reasoning) {
	std::cerr << "An error occured during execution. Reason:\n" + reasoning + "\nProgram terminated.";
	throw std::runtime_error("Illegal program can not be executed.");
}

void append_str_buf(std::stringbuf& buf, const std::string& str)
{
	buf.sputn(str.c_str(), static_cast<std::streamsize>(str.length()));
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


std::string str_to_cpp(const char* copy)
{
	std::string result{ copy };
	delete[] (copy);
	return result;
}

Value::Value(Logic value) : value(value) {}
Value::Value(Number value) : value(value) {}
Value::Value(Text text) : value(text) {}


Variable::Variable(std::string name, Value init_value)
	: name(std::move(name))
	, value(std::move(init_value))
{
}

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



Function::Function(std::string name, StatementNode* body, Signature signature)
	: name(std::move(name))
	, body(body)
	, signature(std::move(signature))
{
}

auto Function::call(ExecutionScopedState& context, const std::vector<std::string>& args) const -> std::optional<Value>
{
	bool termination_token = false;
	std::optional<Value> result;
	ExecutionScopedState call_context{ &context, &termination_token, &result };

	// REBIND ARGS
	for (size_t i = 0; i < args.size(); ++i) 
	{
		const auto& arg = args.at(i);
		const Value* value = context.try_get_var_value(arg);

		if (value == nullptr) {
			terminate_illegal_program("Function argument does not exist.");
		}

		Variable variable{ signature.at(i), *value };

		call_context.declare_variable(std::move(variable));
	}

	body->execute(call_context);

	return result;
}

auto Function::get_name() const -> const std::string&
{
	return this->name;
}


namespace ValueVisitors
{
	struct ValuePrinter final
	{
		std::string* target;

		void operator()(const Value::Logic boolean) const {
			*target = "Logic: " + std::string(boolean ? "True" : "False");
		}

		void operator()(const Value::Number number) const {
			*target = "Number: " + std::to_string(number);
		}

		void operator()(const Value::Text text) const {
			*target = "Text: " + text;
		}
	};

	struct BinaryOperation final
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
			};

			*result = Value(calc());
		}

		void operator()(ComparisonOperation comparison_operation)
		{
			const Value::Logic* logicL = left_value->try_get<Value::Logic>();
			const Value::Number* numberL = left_value->try_get<Value::Number>();

			if (logicL) 
			{
				const Value::Logic* logicR = right_value->try_get<Value::Logic>();

				if (logicR == nullptr) {
					terminate_illegal_program("Logic value must be compared with other logic value.");
				}

				bool evaluated;
				switch (comparison_operation) {
					case ComparisonOperation::Equality:
						evaluated = *logicR == *logicL;
						break;
					case ComparisonOperation::Inequality:
						evaluated = *logicR != *logicL;
						break;
					default:
						terminate_illegal_program("Logic value may not be a subject of this comparison operation.");
				}
				
				*result = Value(evaluated);
			}
			else if (numberL) 
			{
				const Value::Number* numberR = right_value->try_get<Value::Number>();

				if (numberR == nullptr) {
					terminate_illegal_program("Number value must be compared with other number value.");
				}

				bool evaluated;
				switch (comparison_operation) {
					case ComparisonOperation::Equality:
						evaluated = (*numberL) == (*numberR);
						break;
					case ComparisonOperation::Inequality:
						evaluated = (*numberL) != (*numberR);
						break;
					case ComparisonOperation::Less:
						evaluated = (*numberL) < (*numberR);
						break;
					case ComparisonOperation::LessOrEqual:
						evaluated = (*numberL) <= (*numberR);
						break;
					case ComparisonOperation::More:
						evaluated = (*numberL) > (*numberR);
						break;
					case ComparisonOperation::MoreOrEqual:
						evaluated = (*numberL) >= (*numberR);
						break;

					default:
						terminate_illegal_program("Unknown comparison operation.");
				}

				*result = Value(evaluated);
			}
			else 
			{
				terminate_illegal_program("The type can not be a subject of comparison operator.");
			}
		}
	};

	struct Reassignment final
	{
		Value* target;

		void operator()(const Value::Logic boolean) const {
			*target = Value(boolean);
		}

		void operator()(const Value::Number number) const {
			*target = Value(number);
		}

		void operator()(const Value::Text text) const {
			*target = Value(text);
		}
	};
}


void Value::reassign(const Value& src)
{
	if (src.value.index() != this->value.index())
	{
		terminate_illegal_program("Variable type can not be changed.");
	}

	ValueVisitors::Reassignment reassignment;
	reassignment.target = this;

	std::visit(reassignment, src.value);
}



// Creates a predicate to be used to search variable by its name.
auto get_var_name_predicate(std::string_view name)
{
	return [name](const Variable& var) -> bool {
		return var.get_name() == name;
	};
}


ExecutionScopedState::ExecutionScopedState(bool* termination_token, std::optional<Value>* result)
	: result(result)
	, termination_token(termination_token)
{

}

ExecutionScopedState::ExecutionScopedState(ExecutionScopedState* parent_state, bool* termination_token, std::optional<Value>* result)
	: parent_state(parent_state)
	, result(result)
	, termination_token(termination_token)
	, level(parent_state->level + 1)
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

auto ExecutionScopedState::try_get_function(std::string_view name) const -> const Function*
{
	auto result = std::find_if(
		functions.begin(),
		functions.end(),
		[name](const Function& func) -> bool
		{
			return name == func.get_name();
		}
	);

	const bool found = result != functions.end();

	if (!found && parent_state) {
		if (const Function* parent_function = parent_state->try_get_function(name)) {
			return parent_function;
		}
	}

	return found ? &(*result) : nullptr;
}

void ExecutionScopedState::declare_variable(Variable&& variable)
{
	const auto result = std::find_if(
		variables.begin(),
		variables.end(),
		get_var_name_predicate(variable.get_name())
	);

	if (result != variables.end()) {
		terminate_illegal_program("Value with given name is already declared.");
	}

	this->variables.emplace_back(std::move(variable));
}

void ExecutionScopedState::declare_function(Function&& function)
{
	auto name = function.get_name();

	const auto result = std::find_if(
		functions.begin(),
		functions.end(),
		[name](const Function& func) -> bool
		{
			return name == func.get_name();
		}
	);

	if (result != functions.end()) {
		terminate_illegal_program("Function with given name is already declared.");
	}

	this->functions.emplace_back(std::move(function));
}

void ExecutionScopedState::set_result(Value&& value)
{
	if (result->has_value()) {
		terminate_illegal_program("The algorithm has already declared returned value.");
	}

	this->result->emplace(std::move(value));
}

auto ExecutionScopedState::get_result() const -> const std::optional<Value>&
{
	return *this->result;
}

auto ExecutionScopedState::get_result_target() -> std::optional<Value>*
{
	return this->result;
}

void ExecutionScopedState::mark_termination()
{
	*termination_token = true;
}

auto ExecutionScopedState::is_terminated() const -> bool
{
	return *termination_token;
}

auto ExecutionScopedState::get_termination_token() const -> bool*
{
	return termination_token;
}

void ExecutionScopedState::print_summary()
{
	for (const auto& variable : this->variables) {
		std::string str;
		ValueVisitors::ValuePrinter printer{ &str };
		variable.get_value().handle_by_visitor(printer);
		std::cout << variable.get_name() << " = " << str << "\n";
	}
}


AstRoot::AstRoot(StatementNode* head_statement)
	: head_statement(head_statement)
{
}



ArgsListNode::ArgsListNode(std::string name)
	: name(name)
{
}

ArgsListNode::ArgsListNode(std::string name, ArgsListNode* next)
	: name(name)
	, next(std::unique_ptr<ArgsListNode>(next))
{
}


BraceExpressionNode::BraceExpressionNode(ExpressionNode* braced_expression)
	: braced_expression(std::unique_ptr<ExpressionNode>(braced_expression))
{
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


VariableAssignmentNode::VariableAssignmentNode(
	std::string variable_name,
	ExpressionNode* expression,
	const bool reassignment)
	: variable_name(std::move(variable_name))
	, expression(std::unique_ptr<ExpressionNode>(expression))
	, is_reassignment(reassignment)
{
}

MultiStatementsNode::MultiStatementsNode(
	StatementNode* left_statement, 
	StatementNode* right_statement)
	: left_statement(std::unique_ptr<StatementNode>(left_statement))
	, right_statement(std::unique_ptr<StatementNode>(right_statement))
{
}

BodyNode::BodyNode(StatementNode* body_statement)
	: body_statement(std::unique_ptr<StatementNode>(body_statement))
{
}

ConditionalStatementNode::ConditionalStatementNode(
	ExpressionNode* condition,
	StatementNode* statement,
	const bool repeating)
	: condition(std::unique_ptr<ExpressionNode>(condition))
	, statement(std::unique_ptr<StatementNode>(statement))
	, repeating(repeating)
{
}


FunctionDeclarationNode::FunctionDeclarationNode(
	std::string name, 
	StatementNode* body_node,
	ArgsListNode* args)
	: name(std::move(name))
	, body(std::unique_ptr<StatementNode>(body_node))
	, args(std::unique_ptr<ArgsListNode>(args))
{
}

FunctionCallNode::FunctionCallNode(
	std::string name,
	ArgsListNode* args)
	: name(std::move(name))
	, args(std::unique_ptr<ArgsListNode>(args))
{
}

PrintNode::PrintNode(std::string name) : name(std::move(name))
{
}


auto BraceExpressionNode::evaluate(const ExecutionScopedState& execution_scoped_state) -> Value
{
	return braced_expression->evaluate(execution_scoped_state);
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
	execution_scoped_state.mark_termination();
}


void AstNode::print_padding(std::stringbuf& buf, const int32_t depth) const
{
	buf.sputn("\n", 1);
	for (int i = 0; i < depth; ++i) {
		buf.sputn("-", 1);
	}
}

void AstRoot::execute()
{
	bool termination_token = false;
	std::optional<Value> result;
	ExecutionScopedState execution_state{ &termination_token, &result };

	this->head_statement->execute(execution_state);

	if (result.has_value()) {
		std::string str;
		ValueVisitors::ValuePrinter printer{ &str };

		result.value().handle_by_visitor(printer);
		std::cout << "Executed with result: " << str << "\n";

	} else {
		std::cout << "Executed without result.\n";
	}

	execution_state.print_summary();
}

void VariableAssignmentNode::execute(ExecutionScopedState& context) const
{
	if (this->is_reassignment) // 
	{
		Value* value = context.try_get_var_value(this->variable_name);

		if (value == nullptr) {
			terminate_illegal_program("The value does not exist!");
		}

		const Value var_value = this->expression->evaluate(context);

		value->reassign(var_value);
	}
	else // New Variable
	{
		Value var_value = this->expression->evaluate(context);
		Variable variable{ this->variable_name, std::move(var_value) };
		context.declare_variable(std::move(variable));
	}
}

void MultiStatementsNode::execute(ExecutionScopedState& context) const
{
	if (context.is_terminated()) {
		return;
	}

	this->left_statement->execute(context);

	if (context.is_terminated()) {
		return;
	}

	this->right_statement->execute(context);
}

void BodyNode::execute(ExecutionScopedState& context) const
{
	this->body_statement->execute(context);
}

void ConditionalStatementNode::execute(ExecutionScopedState& parent_context) const
{
	auto should_continue = [&]() -> bool
	{
		// This is a hack.
		// With current architecture, EVERY branching must manually check for termination!
		if (parent_context.is_terminated()) {
			return false;
		}

		Value val = this->condition->evaluate(parent_context);
		const bool* value_ptr = val.try_get<bool>();

		if (value_ptr == nullptr) {
			terminate_illegal_program("Expression does not evaluate to boolean.");
		}

		return *value_ptr;
	};

	constexpr auto iteration_cap = 1 << 13;
	const auto max_iteration_count = this->repeating ? iteration_cap : 1;


	for (int i = 0; i < max_iteration_count; ++i) 
	{
		ExecutionScopedState conditional_context{
			&parent_context,
			parent_context.get_termination_token(),
			parent_context.get_result_target()
		};

		if (!should_continue()) {
			return;
		}

		this->statement->execute(conditional_context);
	}

	if (this->repeating) {
		terminate_illegal_program("Iteration count exceeded the limit.");
	}
}


void FunctionDeclarationNode::execute(ExecutionScopedState& context) const
{
	const std::vector<std::string> args = this->args->get_list();
	context.declare_function(Function{ this->name, this->body.get(), args });
}


auto FunctionCallNode::call(const ExecutionScopedState& context) const -> std::optional<Value>
{
	const Function* function = context.try_get_function(this->name);

	if (function == nullptr) {
		terminate_illegal_program("Function is not recognized.");
	}

	const std::vector<std::string> args = this->args->get_list();
	std::optional<Value> value = function->call(const_cast<ExecutionScopedState&>(context), args); //TODO

	return value;
}

auto FunctionCallNode::evaluate(const ExecutionScopedState& context) -> Value
{
	std::optional<Value> result = call(context);

	if (!result.has_value()) {
		terminate_illegal_program("Function does not return anything.");
	}

	return result.value();
}

void FunctionCallNode::execute(ExecutionScopedState& context) const
{
	call(context);
}

void PrintNode::execute(ExecutionScopedState& context) const
{
	const Value* v = context.try_get_var_value(this->name);

	if (v == nullptr) {
		std::cout << this->name << " does not exist\n";
	}

	std::string target;
	const ValueVisitors::ValuePrinter printer{ &target };
	v->handle_by_visitor(printer);

	std::cout << this->name << " = " << target << "\n";
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

	std::cout << buffer.str() << "\n";
}


void ArgsListNode::print(std::stringbuf& buf, int32_t depth) const
{
}


void ArgsListNode::append_list(std::vector<std::string>& list)
{
	list.emplace_back(this->name);

	if (next) {
		next->append_list(list);
	}
}

auto ArgsListNode::get_list() -> std::vector<std::string>
{
	std::vector<std::string> args;

	append_list(args);

	return args;
}


void BraceExpressionNode::print(std::stringbuf& buf, const int32_t depth) const
{
	this->braced_expression->print(buf, depth + 1);
}

void LiteralNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	std::string str;
	this->value.handle_by_visitor(ValueVisitors::ValuePrinter{ &str });
	append_str_buf(buf, str);
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

	append_str_buf(buf, "Ref: ");
	append_str_buf(buf, name);
}

void ResultNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	append_str_buf(buf, "Return");

	this->result_expression->print(buf, depth + 1);
}

void VariableAssignmentNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	if (is_reassignment) {
		append_str_buf(buf, this->variable_name);
		append_str_buf(buf, " := ...");
	} else {
		append_str_buf(buf, this->variable_name);
		append_str_buf(buf, " = ...");
	}
}

void MultiStatementsNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	append_str_buf(buf, "L/R:");

	this->right_statement->print(buf, depth + 1);
	this->left_statement->print(buf, depth + 1);
}

void BodyNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	append_str_buf(buf, "Body");

	this->body_statement->print(buf, depth + 1);
}

void ConditionalStatementNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);

	append_str_buf(buf, "Conditional");

	this->condition->print(buf, depth + 1);

	this->condition->print(buf, depth + 1);
}

void FunctionDeclarationNode::print(std::stringbuf& buf, const int32_t depth) const
{
	print_padding(buf, depth);
}

void FunctionCallNode::print(std::stringbuf& buf, int32_t depth) const
{
}

void PrintNode::print(std::stringbuf& buf, int32_t depth) const
{
}
