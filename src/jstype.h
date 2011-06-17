// heuristic js type
#ifndef _AZ_JSTYPE_H_
#define _AZ_JSTYPE_H_
#include <iv/detail/functional.h>
namespace az {

enum JSType {
  // initial value
  TYPE_NOT_SEARCHED = 0,

  // Any Type
  TYPE_ANY,

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
  TYPE_USER_OBJECT
};

std::array<const char*, 12> kTypeName = { {
  "NotSearched",
  "Any",
  "String",
  "Number",
  "Boolean",
  "Undefined",
  "Null",
  "Function",
  "RegExp",
  "Array",
  "Object",
  "UserObject"
} };

const char* GetTypeName(JSType type) {
  return kTypeName[type];
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

} IV_HASH_NAMESPACE_END
#endif  // _AZ_JSTYPE_H_
