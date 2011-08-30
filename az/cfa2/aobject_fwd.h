#ifndef AZ_CFA2_AOBJECT_FWD_H_
#define AZ_CFA2_AOBJECT_FWD_H_
#include <iv/noncopyable.h>
#include <iv/space.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/result.h>
#include <az/cfa2/builtins_fwd.h>
namespace az {

class Completer;

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
      attributes_(A::W | A::E | A::C) {
  }

  explicit AProp(const AVal& val, int type)
    : value_(val),
      attributes_(type & (A::W | A::E | A::C)) {
  }

  const AVal& value() const {
    return value_;
  }

  void Merge(const AVal& val) {
    value_ |= val;
  }

  bool IsWritable() const {
    return attributes_ & A::W;
  }

  bool IsEnumerable() const {
    return attributes_ & A::E;
  }

  bool IsConfigurable() const {
    return attributes_ & A::C;
  }

 private:
  AVal value_;
  int attributes_;
};

// abstract object
class AObject
  : private iv::core::Noncopyable<AObject>,
    public iv::core::SpaceObject {
 public:
  typedef std::unordered_map<Symbol, AProp> Properties;
  AObject()
    : proto_(AVAL_NOBASE),
      number_(),
      string_(),
      function_(NULL),
      builtin_(NULL),
      properties_() {
  }

  AObject(FunctionLiteral* function, const AVal& proto)
    : proto_(proto),
      number_(),
      string_(),
      function_(function),
      builtin_(NULL),
      properties_() {
  }

  AObject(Builtin builtin, const AVal& proto)
    : proto_(proto),
      number_(),
      string_(),
      function_(NULL),
      builtin_(builtin),
      properties_() {
  }

  explicit AObject(const AVal& proto)
    : proto_(proto),
      number_(),
      string_(),
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
      if (number_ && IsArrayIndexSymbol(name)) {
        return *number_;
      }
      if (string_) {
        return *string_;
      }
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

  inline iv::core::UString ToTypeString(Heap* heap, std::unordered_set<const AObject*>* already_searched) const;

  bool IsFunction() const {
    return function_ || builtin_;
  }

  inline AVal GetNumberProperty(Heap* heap);
  inline AVal GetNumberPropertyImpl(std::unordered_set<const AObject*>* already_searched) const;
  inline void MergeNumberProperty(Heap* heap);
  inline void UpdateNumberProperty(Heap* heap, const AVal& val);
  inline AVal GetStringProperty(Heap* heap);
  inline AVal GetStringPropertyImpl(std::unordered_set<const AObject*>* already_searched) const;
  inline void MergeStringProperty(Heap* heap);
  inline void UpdateStringProperty(Heap* heap, const AVal& val);

  inline void Complete(Heap* heap, Completer* completer) const;

 private:
  AVal proto_;
  std::shared_ptr<AVal> number_;
  std::shared_ptr<AVal> string_;
  FunctionLiteral* function_;
  Builtin builtin_;
  Properties properties_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_AOBJECT_FWD_H_
