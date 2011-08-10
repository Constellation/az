#ifndef _AZ_CFA2_AOBJECT_FWD_H_
#define _AZ_CFA2_AOBJECT_FWD_H_
#include <iv/noncopyable.h>
#include <iv/space.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/result.h>
#include <az/cfa2/builtins_fwd.h>
namespace az {
namespace cfa2 {

namespace A {
  enum Attribute {
    NONE = 0,
    WRITABLE = 1,
    ENUMERABLE = 2,
    CONFIGURABLE = 4,

    // short hand
    N = 0,
    W = 1,
    E = 2,
    C = 4
  };
}  // namespace A

// abstract property
class AProp {
 public:
  explicit AProp(const AVal& val)
    : value_(val),
      writable_(true),
      enumerable_(true),
      configurable_(true) {
  }

  explicit AProp(const AVal& val, int type)
    : value_(val),
      writable_(type & A::W),
      enumerable_(type & A::E),
      configurable_(type & A::C) {
  }

  const AVal& value() const {
    return value_;
  }

  void Merge(const AVal& val) {
    value_ |= val;
  }

 private:
  // TODO(Constellation) use bitset
  AVal value_;
  bool writable_;
  bool enumerable_;
  bool configurable_;
};

// abstract object
class AObject
  : private iv::core::Noncopyable<AObject>,
    public iv::core::SpaceObject {
 public:
  typedef std::unordered_map<Symbol, AProp> Properties;
  AObject()
    : proto_(AVAL_NOBASE),
      function_(NULL),
      builtin_(NULL),
      properties_() {
  }

  AObject(FunctionLiteral* function, const AVal& proto)
    : proto_(proto),
      function_(function),
      builtin_(NULL),
      properties_() {
  }

  AObject(Builtin builtin, const AVal& proto)
    : proto_(proto),
      function_(NULL),
      builtin_(builtin),
      properties_() {
  }

  explicit AObject(const AVal& proto)
    : proto_(proto),
      function_(NULL),
      builtin_(NULL),
      properties_() {
  }

  Builtin builtin() const {
    return builtin_;
  }

  FunctionLiteral* function() const {
    return function_;
  }

  void AddProperty(Symbol name, const AProp& prop) {
    properties_.insert(std::make_pair(name, prop));
  }

  inline void UpdateProperty(Heap* heap, Symbol name, const AVal& val);

  inline void UpdatePrototype(Heap* heap, const AVal& val);

  bool GetOwnProperty(Symbol name, AVal* val) const {
    const Properties::const_iterator it = properties_.find(name);
    if (it != properties_.end()) {
      *val = it->second.value();
      return true;
    }
    *val = AVal(AVAL_NOBASE);
    return false;
  }

  AVal GetProperty(Symbol name) const {
    AVal result;
    if (GetOwnProperty(name, &result)) {
      return result;
    } else {
      // Lookup [[Prototype]]
      std::unordered_set<const AObject*> already_searched;
      already_searched.insert(this);
      return proto_.GetPropertyImpl(name, &already_searched);
    }
  }

  AVal GetPropertyImpl(Symbol name, std::unordered_set<const AObject*>* already_searched) const {
    AVal result;
    if (GetOwnProperty(name, &result)) {
      already_searched->insert(this);
      return result;
    } else {
      already_searched->insert(this);
      return proto_.GetPropertyImpl(name, already_searched);
    }
  }

  iv::core::UString ToTypeString(std::unordered_set<const AObject*>* already_searched) const {
    if (already_searched->find(this) != already_searched->end()) {
      return iv::core::ToUString("any");
    } else {
      already_searched->insert(this);
    }
    if (IsFunction()) {
      // TODO(Constellation) implement it
      return iv::core::ToUString("function");
    }
    const AVal constructor = GetProperty(Intern("constructor"));
    if (constructor.IsUndefined()) {
      // constructor not found
      return iv::core::ToUString("Global");
    }
    return iv::core::ToUString("Object");
  }

  bool IsFunction() const {
    return function_ || builtin_;
  }

 private:
  AVal proto_;
  FunctionLiteral* function_;
  Builtin builtin_;
  Properties properties_;
};



} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AOBJECT_FWD_H_
