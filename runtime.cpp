#include "runtime.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <sstream>

using namespace std;

namespace runtime {

	namespace {
		const string STR_METHOD = "__str__"s;
		const string EQ_METHOD = "__eq__"s;
		const string LT_METHOD = "__lt__"s;
	} // namespace

	ObjectHolder::ObjectHolder(std::shared_ptr<Object> data)
		: data_(std::move(data)) {
	}

	void ObjectHolder::AssertIsValid() const {
		assert(data_ != nullptr);
	}

	ObjectHolder ObjectHolder::Share(Object& object) {
		// Возвращаем невладеющий shared_ptr (его deleter ничего не делает)
		return ObjectHolder(std::shared_ptr<Object>(&object, [](auto* /*p*/) { /* do nothing */ }));
	}

	ObjectHolder ObjectHolder::None() {
		return ObjectHolder();
	}

	Object& ObjectHolder::operator*() const {
		AssertIsValid();
		return *Get();
	}

	Object* ObjectHolder::operator->() const {
		AssertIsValid();
		return Get();
	}

	Object* ObjectHolder::Get() const {
		return data_.get();
	}

	ObjectHolder::operator bool() const {
		return Get() != nullptr;
	}

	bool IsTrue(const ObjectHolder& object) {
		const Number* number_ptr = object.TryAs<Number>();
		if (number_ptr) {
			return number_ptr->GetValue() != 0;
		}
		const String* string_ptr = object.TryAs<String>();
		if (string_ptr) {
			return !string_ptr->GetValue().empty();
		}
		const Bool* bool_ptr = object.TryAs<Bool>();
		if (bool_ptr) {
			return bool_ptr->GetValue();
		}
		return false;
	}

	void ClassInstance::Print(std::ostream& os, Context& context) {
		if (HasMethod(STR_METHOD, 0u)) {
			Call(STR_METHOD, {}, context)->Print(os, context);
		}
		else {
			os << this;
		}
	}

	bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
		const Method* method_ptr = cls_.GetMethod(method);
		return method_ptr && method_ptr->formal_params.size() == argument_count;
	}

	Closure& ClassInstance::Fields() {
		return fields_;
	}

	const Closure& ClassInstance::Fields() const {
		return fields_;
	}

	ClassInstance::ClassInstance(const Class& cls)
		: cls_(cls) {
	}

	ObjectHolder ClassInstance::Call(const std::string& method,
		const std::vector<ObjectHolder>& actual_args,
		Context& context) {
		if (!HasMethod(method, actual_args.size())) {
			throw std::runtime_error("Class "s + cls_.GetName() + " does not implement "s + method + " method"s);
		}
		const Method* method_ptr = cls_.GetMethod(method);
		Closure closure;
		closure["self"s] = ObjectHolder::Share(*this);
		for (auto i = 0u; i < actual_args.size(); ++i) {
			closure[method_ptr->formal_params[i]] = actual_args[i];
		}
		return method_ptr->body->Execute(closure, context);
	}

	Class::Class(std::string name, std::vector<Method> methods, const Class* parent)
		: name_(std::move(name))
		, methods_(std::move(methods))
		, parent_(parent) {
	}

	const Method* Class::GetMethod(const std::string& name) const {
		const auto method_it = std::find_if(methods_.begin(), methods_.end(),
			[&name](const Method& m) {
				return m.name == name;
			}
		);
		if (method_it != methods_.end()) {
			return &(*method_it);
		}
		else if (!parent_) {
			return nullptr;
		}
		return parent_->GetMethod(name);
	}

	void Class::Print(ostream& os, Context& /*context*/) {
		os << "Class "s << name_;
	}

	void Bool::Print(std::ostream& os, [[maybe_unused]] Context& context) {
		os << (GetValue() ? "True"sv : "False"sv);
	}

	bool Equal(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		const Number* num_lhs_ptr = lhs.TryAs<Number>();
		const Number* num_rhs_ptr = rhs.TryAs<Number>();
		if (num_lhs_ptr && num_rhs_ptr) {
			return num_lhs_ptr->GetValue() == num_rhs_ptr->GetValue();
		}
		const String* str_lhs_ptr = lhs.TryAs<String>();
		const String* str_rhs_ptr = rhs.TryAs<String>();
		if (str_lhs_ptr && str_rhs_ptr) {
			return str_lhs_ptr->GetValue() == str_rhs_ptr->GetValue();
		}
		const Bool* bool_lhs_ptr = lhs.TryAs<Bool>();
		const Bool* bool_rhs_ptr = rhs.TryAs<Bool>();
		if (bool_lhs_ptr && bool_rhs_ptr) {
			return bool_lhs_ptr->GetValue() == bool_rhs_ptr->GetValue();
		}
		ClassInstance* cls_instance_ptr = lhs.TryAs<ClassInstance>();
		if (cls_instance_ptr && cls_instance_ptr->HasMethod(EQ_METHOD, 1u)) {
			return cls_instance_ptr->Call(EQ_METHOD, { rhs }, context).TryAs<Bool>()->GetValue();
		}
		if (!bool(lhs) && !bool(rhs)) {
			return true;
		}
		throw std::runtime_error("Cannot compare objects for equality"s);
	}

	bool Less(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		const Number* num_lhs_ptr = lhs.TryAs<Number>();
		const Number* num_rhs_ptr = rhs.TryAs<Number>();
		if (num_lhs_ptr && num_rhs_ptr) {
			return num_lhs_ptr->GetValue() < num_rhs_ptr->GetValue();
		}
		const String* str_lhs_ptr = lhs.TryAs<String>();
		const String* str_rhs_ptr = rhs.TryAs<String>();
		if (str_lhs_ptr && str_rhs_ptr) {
			return str_lhs_ptr->GetValue() < str_rhs_ptr->GetValue();
		}
		const Bool* bool_lhs_ptr = lhs.TryAs<Bool>();
		const Bool* bool_rhs_ptr = rhs.TryAs<Bool>();
		if (bool_lhs_ptr && bool_rhs_ptr) {
			return bool_lhs_ptr->GetValue() < bool_rhs_ptr->GetValue();
		}
		ClassInstance* cls_instance_ptr = lhs.TryAs<ClassInstance>();
		if (cls_instance_ptr && cls_instance_ptr->HasMethod(LT_METHOD, 1u)) {
			return cls_instance_ptr->Call(LT_METHOD, { rhs }, context).TryAs<Bool>()->GetValue();
		}
		throw std::runtime_error("Cannot compare objects for less"s);
	}

	bool NotEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return !Equal(lhs, rhs, context);
	}

	bool Greater(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return !Less(lhs, rhs, context) && !Equal(lhs, rhs, context);
	}

	bool LessOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return Less(lhs, rhs, context) || Equal(lhs, rhs, context);
	}

	bool GreaterOrEqual(const ObjectHolder& lhs, const ObjectHolder& rhs, Context& context) {
		return !Less(lhs, rhs, context);
	}

}  // namespace runtime