// heuristic js type
#ifndef _AZ_JSTYPE_H_
#define _AZ_JSTYPE_H_
#include <utility>
#include <iv/detail/functional.h>
namespace az {

enum JSType {
  // Primitive JSTypes
  TYPE_STRING,
  TYPE_NUMBER,
  TYPE_BOOLEAN,
  TYPE_UNDEFINED,
  TYPE_NULL,

  // Literally Analyzable JSTypes
  TYPE_FUNCTION,
  TYPE_REGEXP,
  TYPE_ARRAY,
  TYPE_OBJECT,
  // TYPE_USER_OBJECT

  TYPE_ANY
};

std::array<const char*, 11> kTypeName = { {
  "String",
  "Number",
  "Boolean",
  "Undefined",
  "Null",
  "Function",
  "RegExp",
  "Array",
  "Object",
  "Any",
} };

class JSObject;

class Type {
 public:
  Type(JSType type, std::shared_ptr<JSObject> obj)
    : type_(type),
      obj_(obj) {
  }

  explicit Type(JSType type)
    : type_(type),
      obj_(std::shared_ptr<JSObject>()) {
  }

  Type()
    : type_(TYPE_ANY),
      obj_(std::shared_ptr<JSObject>()) {
  }

  JSType type() const {
    return type_;
  }

  std::shared_ptr<JSObject> obj() const {
    return obj_;
  }

  friend bool operator==(const Type& lhs, const Type& rhs) {
    return lhs.type_ == rhs.type_ && lhs.obj_ == rhs.obj_;
  }

  friend bool operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
  }

 private:
  JSType type_;
  std::shared_ptr<JSObject> obj_;
};

class JSObject {
 private:
  std::unordered_map<Symbol, JSType> properties_;
};

typedef std::unordered_set<Type> TypeSet;

inline bool IsObjectType(JSType type) {
  return
      type == TYPE_FUNCTION ||
      type == TYPE_REGEXP ||
      type == TYPE_ARRAY ||
      type == TYPE_OBJECT;
}

}  // namespace az
namespace IV_HASH_NAMESPACE_START {

template<>
struct hash<az::JSType>
  : public std::unary_function<az::JSType, std::size_t> {
  result_type operator()(const argument_type& x) const {
    return hash<int>()(static_cast<int>(x));
  }
};

template<>
struct hash<az::Type>
  : public std::unary_function<az::Type, std::size_t> {
  result_type operator()(const argument_type& x) const {
    return hash<az::JSType>()(x.type()) + hash<const az::JSObject*>()(x.obj().get());
  }
};

} IV_HASH_NAMESPACE_END
namespace az {

class AType {
 public:
  AType(Type type)
    : primary_(type),
      set_() {
    if (primary_.type() != TYPE_ANY) {
      set_.insert(primary_);
    }
  }

  AType(JSType type)
    : primary_(type),
      set_() {
    if (type != TYPE_ANY) {
      set_.insert(primary_);
    }
  }

  AType()
    : primary_(TYPE_ANY),
      set_() {
  }

  bool IsPrimaryTyped(JSType type) const {
    if (!IsVacantType()) {
      if (type == primary_.type()) {
        return true;
      }
    }
    return false;
  }

  bool IsObjectType() const {
    if (!IsVacantType()) {
      return ::az::IsObjectType(primary_.type());
    }
    return false;
  }

  bool IsVacantType() const {
    return set_.empty();
  }

  const Type& primary() const {
    return primary_;
  }

  const TypeSet& set() const {
    return set_;
  }

  TypeSet* set() {
    return &set_;
  }

  static AType Merged(const AType& lhs, const AType& rhs) {
    if (lhs.IsVacantType() && rhs.IsVacantType()) {
      return AType();
    }
    return AType(lhs, rhs);
  }

 private:
  AType(const AType& lhs, const AType& rhs)
    : primary_(),
      set_() {
    if (!rhs.IsVacantType()) {
      primary_ = rhs.primary();
    } else {
      assert(!lhs.IsVacantType());
      primary_ = lhs.primary();
    }
    for (TypeSet::const_iterator it = lhs.set_.begin(),
         last = lhs.set_.end(); it != last; ++it) {
      if (it->type() != TYPE_ANY) {
        set_.insert(*it);
      }
    }
    for (TypeSet::const_iterator it = rhs.set_.begin(),
         last = rhs.set_.end(); it != last; ++it) {
      if (it->type() != TYPE_ANY) {
        set_.insert(*it);
      }
    }
  }
  Type primary_;
  TypeSet set_;
};

inline bool IsConservativeEqualType(const AType& lhs, const AType& rhs) {
  if (!lhs.set().empty() && !rhs.set().empty()) {
    if (lhs.primary() != rhs.primary()) {
      return false;
    }
  }
  return true;
}

inline const char* GetTypeName(JSType type) {
  return kTypeName[type];
}

inline const char* GetTypeName(AType type) {
  return kTypeName[type.primary().type()];
}

}  // namespace az
#endif  // _AZ_JSTYPE_H_
