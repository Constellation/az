#ifndef _IV_TYPE_HEAP_H_
#define _IV_ASTE_HEAP_H_
#include "ast_fwd.h"
#include "jstype.h"
namespace az {

struct StringType { };
struct NumberType { };

class TypeVariable {
 public:
  TypeVariable(StringType str)
    : type_tag_(TYPE_STRING),
      prototype_(NULL) {
  }

 private:
  JSType type_tag_;
  const FunctionLiteral* prototype_;
};

typedef std::unordered_set<TypeVariable> TypeVariables;

class TypeHeap {
 public:
 private:
  std::unordered_map<
      const Expression*,
      std::shared_ptr<TypeVariables> > heap_;
};

}  // namespace az
#endif  // _IV_ASTE_HEAP_H_
