#ifndef _AZ_CFA2_AOBJECT_H_
#define _AZ_CFA2_AOBJECT_H_
#include <iv/noncopyable.h>
#include <iv/space.h>
#include <az/cfa2/aval_fwd.h>
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
    value_.Join(val);
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
    : proto_(),
      function_(NULL),
      properties_() {
  }

  AObject(FunctionLiteral* function, const AVal& proto)
    : proto_(proto),
      function_(function),
      properties_() {
  }

  explicit AObject(const AVal& proto)
    : proto_(proto),
      function_(NULL),
      properties_() {
  }

  void AddProperty(Symbol name, const AProp& prop) {
    properties_.insert(std::make_pair(name, prop));
  }

  void UpdateProperty(Symbol name, AVal val) {
    const Properties::iterator it = properties_.find(name);
    if (it != properties_.end()) {
      it->second.Merge(val);
    } else {
      AddProperty(name, AProp(val));
    }
  }

  bool GetOwnProperty(Symbol name, AVal* val) {
    const Properties::const_iterator it = properties_.find(name);
    if (it != properties_.end()) {
      *val = it->second.value();
      return true;
    }
    *val = AVal();  // undefined
    return false;
  }

  AVal GetProperty(Symbol name) {
    AVal result;
    if (GetOwnProperty(name, &result)) {
      return result;
    } else {
      // not found in this object
      // return proto_.LookupProtoChain(name);
      return AVal();
    }
  }
 private:
  AVal proto_;
  FunctionLiteral* function_;
  Properties properties_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AOBJECT_H_
