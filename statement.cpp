#include "statement.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace ast {

	using runtime::Closure;
	using runtime::Context;
	using runtime::ObjectHolder;

	namespace {
		const string ADD_METHOD = "__add__"s;
		const string INIT_METHOD = "__init__"s;
	}  // namespace

	ObjectHolder Assignment::Execute(Closure& closure, Context& context) {
		return closure[var_] = rv_->Execute(closure, context);
	}

	Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv)
		: var_(std::move(var))
		, rv_(std::move(rv)) {
	}

	VariableValue::VariableValue(const std::string& var_name)
		: dotted_ids_{ var_name } {
	}

	VariableValue::VariableValue(std::vector<std::string> dotted_ids)
		: dotted_ids_(std::move(dotted_ids)) {
	}

	ObjectHolder VariableValue::Execute(Closure& closure, Context& /*context*/) {
		Closure* closure_ptr = &closure;
		for (size_t i = 0u; i + 1u < dotted_ids_.size(); ++i) {
			if (!closure_ptr->count(dotted_ids_[i])) {
				throw std::runtime_error("Variable "s + dotted_ids_[i] + " not found"s);
			}
			closure_ptr = &(*closure_ptr)[dotted_ids_[i]].TryAs<runtime::ClassInstance>()->Fields();
		}
		if (!closure_ptr->count(dotted_ids_.back())) {
			throw std::runtime_error("Variable "s + dotted_ids_.back() + " not found"s);
		}
		return (*closure_ptr)[dotted_ids_.back()];
	}

	unique_ptr<Print> Print::Variable(const std::string& name) {
		return std::make_unique<Print>(std::make_unique<VariableValue>(name));
	}

	Print::Print(unique_ptr<Statement> argument) {
		args_.push_back(std::move(argument));
	}

	Print::Print(vector<unique_ptr<Statement>> args)
		: args_(std::move(args)) {
	}

	ObjectHolder Print::Execute(Closure& closure, Context& context) {
		std::stringstream ss;
		if (!args_.empty()) {
			bool first = true;
			for (size_t i = 0u; i < args_.size(); ++i) {
				if (!first) ss << " "s;
				first = false;
				ObjectHolder obj_holder = args_[i]->Execute(closure, context);
				if (obj_holder) {
					obj_holder->Print(ss, context);
				}
				else {
					ss << "None"s;
				}
			}
		}
		ss << "\n"s;
		runtime::String(ss.str()).Print(context.GetOutputStream(), context);
		return ObjectHolder::None();
	}

	MethodCall::MethodCall(std::unique_ptr<Statement> object, std::string method,
		std::vector<std::unique_ptr<Statement>> args)
		: object_(std::move(object))
		, method_(std::move(method))
		, args_(std::move(args)) {
	}

	ObjectHolder MethodCall::Execute(Closure& closure, Context& context) {
		runtime::ClassInstance* cls_instance_ptr = object_->Execute(closure, context).TryAs<runtime::ClassInstance>();
		if (cls_instance_ptr && cls_instance_ptr->HasMethod(method_, args_.size())) {
			std::vector<ObjectHolder> actual_args(args_.size());
			for (size_t i = 0u; i < args_.size(); ++i) {
				actual_args[i] = args_[i]->Execute(closure, context);
			}
			return cls_instance_ptr->Call(method_, actual_args, context);
		}
		else {
			throw std::runtime_error("Object is not a class instance"s);
		}
	}

	ObjectHolder Stringify::Execute(Closure& closure, Context& context) {
		std::stringstream ss;
		ObjectHolder obj_holder = arg_->Execute(closure, context);
		if (obj_holder) {
			obj_holder->Print(ss, context);
		}
		else {
			ss << "None"s;
		}
		return ObjectHolder::Own(runtime::String(ss.str()));
	}

	ObjectHolder Add::Execute(Closure& closure, Context& context) {
		ObjectHolder lhs_obj_holder = lhs_->Execute(closure, context);
		ObjectHolder rhs_obj_holder = rhs_->Execute(closure, context);
		runtime::Number* lhs_num_ptr = lhs_obj_holder.TryAs<runtime::Number>();
		runtime::Number* rhs_num_ptr = rhs_obj_holder.TryAs<runtime::Number>();
		if (lhs_num_ptr && rhs_num_ptr) {
			return ObjectHolder::Own(runtime::Number(lhs_num_ptr->GetValue() + rhs_num_ptr->GetValue()));
		}
		runtime::String* lhs_str_ptr = lhs_obj_holder.TryAs<runtime::String>();
		runtime::String* rhs_str_ptr = rhs_obj_holder.TryAs<runtime::String>();
		if (lhs_str_ptr && rhs_str_ptr) {
			return ObjectHolder::Own(runtime::String(lhs_str_ptr->GetValue() + rhs_str_ptr->GetValue()));
		}
		runtime::ClassInstance* lhs_cls_instance_ptr = lhs_obj_holder.TryAs<runtime::ClassInstance>();
		if (lhs_cls_instance_ptr && lhs_cls_instance_ptr->HasMethod(ADD_METHOD, 1u)) {
			return lhs_cls_instance_ptr->Call(ADD_METHOD, { rhs_obj_holder }, context);
		}
		throw std::runtime_error("Cannot add arguments"s);
	}

	ObjectHolder Sub::Execute(Closure& closure, Context& context) {
		ObjectHolder lhs_obj_holder = lhs_->Execute(closure, context);
		ObjectHolder rhs_obj_holder = rhs_->Execute(closure, context);
		runtime::Number* lhs_num_ptr = lhs_obj_holder.TryAs<runtime::Number>();
		runtime::Number* rhs_num_ptr = rhs_obj_holder.TryAs<runtime::Number>();
		if (lhs_num_ptr && rhs_num_ptr) {
			return ObjectHolder::Own(runtime::Number(lhs_num_ptr->GetValue() - rhs_num_ptr->GetValue()));
		}
		throw std::runtime_error("Cannot subtract arguments"s);
	}

	ObjectHolder Mult::Execute(Closure& closure, Context& context) {
		ObjectHolder lhs_obj_holder = lhs_->Execute(closure, context);
		ObjectHolder rhs_obj_holder = rhs_->Execute(closure, context);
		runtime::Number* lhs_num_ptr = lhs_obj_holder.TryAs<runtime::Number>();
		runtime::Number* rhs_num_ptr = rhs_obj_holder.TryAs<runtime::Number>();
		if (lhs_num_ptr && rhs_num_ptr) {
			return ObjectHolder::Own(runtime::Number(lhs_num_ptr->GetValue() * rhs_num_ptr->GetValue()));
		}
		throw std::runtime_error("Cannot multiply arguments"s);
	}

	ObjectHolder Div::Execute(Closure& closure, Context& context) {
		ObjectHolder lhs_obj_holder = lhs_->Execute(closure, context);
		ObjectHolder rhs_obj_holder = rhs_->Execute(closure, context);
		runtime::Number* lhs_num_ptr = lhs_obj_holder.TryAs<runtime::Number>();
		runtime::Number* rhs_num_ptr = rhs_obj_holder.TryAs<runtime::Number>();
		if (lhs_num_ptr && rhs_num_ptr) {
			if (rhs_num_ptr->GetValue() == 0) {
				throw std::runtime_error("Zero division"s);
			}
			return ObjectHolder::Own(runtime::Number(lhs_num_ptr->GetValue() / rhs_num_ptr->GetValue()));
		}
		throw std::runtime_error("Cannot divide arguments"s);
	}

	ObjectHolder Compound::Execute(Closure& closure, Context& context) {
		for (const auto& statement : statements_) {
			statement->Execute(closure, context);
		}
		return ObjectHolder::None();
	}

	ObjectHolder Return::Execute(Closure& closure, Context& context) {
		throw statement_->Execute(closure, context);
	}

	ClassDefinition::ClassDefinition(ObjectHolder cls)
		: cls_(std::move(cls)) {
	}

	ObjectHolder ClassDefinition::Execute(Closure& closure, Context& /*context*/) {
		runtime::Class* cls_ptr = cls_.TryAs<runtime::Class>();
		closure[cls_ptr->GetName()] = cls_;
		return ObjectHolder::None();
	}

	FieldAssignment::FieldAssignment(VariableValue object, std::string field_name,
		std::unique_ptr<Statement> rv)
		: object_(std::move(object))
		, field_name_(std::move(field_name))
		, rv_(std::move(rv)) {
	}

	ObjectHolder FieldAssignment::Execute(Closure& closure, Context& context) {
		runtime::ClassInstance* cls_instance_ptr = object_.Execute(closure, context).TryAs<runtime::ClassInstance>();
		if (cls_instance_ptr) {
			return cls_instance_ptr->Fields()[field_name_] = rv_->Execute(closure, context);
		}
		return ObjectHolder::None();
	}

	IfElse::IfElse(std::unique_ptr<Statement> condition, std::unique_ptr<Statement> if_body,
		std::unique_ptr<Statement> else_body)
		: condition_(std::move(condition))
		, if_body_(std::move(if_body))
		, else_body_(std::move(else_body)) {
	}

	ObjectHolder IfElse::Execute(Closure& closure, Context& context) {
		if (runtime::IsTrue(condition_->Execute(closure, context))) {
			return if_body_->Execute(closure, context);
		}
		else if (else_body_) {
			return else_body_->Execute(closure, context);
		}
		return ObjectHolder::None();
	}

	ObjectHolder Or::Execute(Closure& closure, Context& context) {
		if (!runtime::IsTrue(lhs_->Execute(closure, context))) {
			return ObjectHolder::Own(runtime::Bool(runtime::IsTrue(rhs_->Execute(closure, context))));
		}
		return ObjectHolder::Own(runtime::Bool(true));
	}

	ObjectHolder And::Execute(Closure& closure, Context& context) {
		if (runtime::IsTrue(lhs_->Execute(closure, context))) {
			return ObjectHolder::Own(runtime::Bool(runtime::IsTrue(rhs_->Execute(closure, context))));
		}
		return ObjectHolder::Own(runtime::Bool(false));
	}

	ObjectHolder Not::Execute(Closure& closure, Context& context) {
		return ObjectHolder::Own(runtime::Bool(!runtime::IsTrue(arg_->Execute(closure, context))));
	}

	Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs)
		: BinaryOperation(std::move(lhs), std::move(rhs))
		, cmp_(std::move(cmp)) {
	}

	ObjectHolder Comparison::Execute(Closure& closure, Context& context) {
		return ObjectHolder::Own(runtime::Bool(cmp_(lhs_->Execute(closure, context), rhs_->Execute(closure, context), context)));
	}

	NewInstance::NewInstance(const runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args)
		: cls_instance_(class_)
		, args_(std::move(args)) {
	}

	NewInstance::NewInstance(const runtime::Class& class_)
		: cls_instance_(class_) {
	}

	ObjectHolder NewInstance::Execute(Closure& closure, Context& context) {
		if (cls_instance_.HasMethod(INIT_METHOD, args_.size())) {
			std::vector<ObjectHolder> actual_args(args_.size());
			for (size_t i = 0u; i < args_.size(); ++i) {
				actual_args[i] = args_[i]->Execute(closure, context);
			}
			cls_instance_.Call(INIT_METHOD, std::move(actual_args), context);
		}
		return ObjectHolder::Share(cls_instance_);
	}

	MethodBody::MethodBody(std::unique_ptr<Statement>&& body)
		: body_(std::move(body)) {
	}

	ObjectHolder MethodBody::Execute(Closure& closure, Context& context) {
		try {
			body_->Execute(closure, context);
		}
		catch (const ObjectHolder& obj_holder) {
			return obj_holder;
		}
		return ObjectHolder::None();
	}

}  // namespace ast