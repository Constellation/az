// CFA2 interpreter
#ifndef _AZ_CFA2_INTERPRETER_H_
#define _AZ_CFA2_INTERPRETER_H_
#include <iv/noncopyable.h>
namespace az {
namespace cfa2 {

// abstract object
class AObject {
 public:
  typedef std::unordered_map<core::UString, AProp> Properties;
  AddProperty(const core::UString name&, const AProp& prop) {
    properties_.insert(std::make_pair(name, prop));
  }

  UpdateProperty(const core::UString name&, AVal val) {
    Properties::const_iterator it = properties_.find(name)
    if (it != properties.end()) {
      it->set_value(AVal::Merge(it->value(), val));
    } else {
      AddProperty(name, AProp(val));
    }
  }

  bool GetOwnProperty(const core::UString& name, AVal* val) {
    Properties::const_iterator it = properties_.find(name)
    if (it != properties.end()) {
      *val = it->value();
      return true;
    }
    *val = AVal();  // undefined
    return false;
  }

  AVal GetProperty(const core::UString& name) {
    AVal result;
    if (GetOwnProperty(name, &result)) {
      return result;
    } else {
      // not found in this object
      return proto.LookupProtoChain(name);
    }
  }
 private:
  AVal proto_;
  FunctionLiteral* function_;
  Properties properties_;
};

enum BaseType {
  AVAL_NOBASE = 0,
  AVAL_NUMBER = 1,
  AVAL_STRING = 2,
  AVAL_BOOL = 4,
  AVAL_UNDEFINED = 8,
  AVAL_NULL = 16
};

// abstract value
class AVal {
 public:
  explicit AVal(BaseType type)
    : base_(type),
      objects_() {
  }

  explicit AVal(AObject* obj)
    : base_(AVAL_NOBASE),
      objects_() {
    objects_.push_back(obj);
  }

  explicit AVal(const core::UString& str)
    : base_(AVAL_STRING),
      str_(str),
      objects_() {
  }

  AVal GetBase() const {
    return AVal(base_);
  }

  AVal ToObject() const {
    if (base_ == AVAL_NOBASE) {
      return *this;
    }
    AVal result(AVAL_NOBASE);
    result.set_objects(objects_);
    if (base_ & AVAL_NUMBER) {
      result.Join(AVAL_NUMBER);
    }
    if (base_ & AVAL_STRING) {
      result.Join(AVAL_STRING);
    }
    if (base & AVAL_BOOL) {
      result.Join(AVAL_BOOL);
    }
  }

  const core::UString& GetStringValue() const {
    return str_;
  }

  bool LookupProtoChain(AVal* result) {
    return false;
  }

  const std::vector<AObject*> objects() const {
    return objects_;
  }

  void set_objects(const std::vector<AObject*> objects) {
    objects_ = objects;
  }

 private:
  BaseType base_;
  core::UString str_;
  std::vector<AObject*> objects_;
};

// abstract property
class AProp {
 public:
  explicit AProp(const AVal& val)
    : value_(val),
      writable_(true),
      enumerable_(true),
      configurable_(true) {
  }

  explicit AProp(const AProp& prop)
    : value_(prop.value_),
      writable_(prop.writable_),
      enumerable_(prop.enumerable_),
      configurable_(prop.configurable_) {
  }

  AVal value() const {
    return value_;
  }

 private:
  AVal value_;
  bool writable_;
  bool enumerable_;
  bool configurable_;
};

typedef std::unordered_set<Type> Types;
inline void NormalizeTypes(Types* types) {
  if (types->find(TYPE_ANY) != types->end()) {
    types->clear();
    types->insert(TYPE_ANY);
  }
}

class Interpreter : private iv::core::Noncopyable<Interpreter> {

 private:
  // CFA2 Variable Stack
  // TODO(Constellation) more precise variable lookup system (railgun/scope.h)
  std::vector<Binding> inner_scope_;
  std::vector<Binding> outer_scope_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_INTERPRETER_H_
