#ifndef _AZ_VARIABLE_TYPE_H_
#define _AZ_VARIABLE_TYPE_H_
#include <iv/detail/unordered_map.h>
#include <iv/detail/unordered_set.h>
#include <iv/detail/memory.h>
#include <iv/ustring.h>
#include "jstype.h"
#include "ast_fwd.h"
namespace az {

typedef std::unordered_set<const AstNode*> DeclaredSet;
typedef std::unordered_set<JSType> TypeSet;

enum VariableType {
  VARIABLE_STACK,     // explicitly defined stack  variable
  VARIABLE_GLOBAL,    // explicitly defined global variable
  VARIABLE_HEAP       // implicitly defined global variable
};

// <VariableType>
//  variable type patterns. STACK, HEAP, GLOBAL
// <Assigned Count>
//  how many times this variable assignment is found
// <JSType>
//  primary type
// <TypeSet>
//  possible type sets
class Var {
 public:
  // FunctionLiteral (arguments) or Declaration or TryStatement (catch block)
  void AddDeclaration(const AstNode* node) {
     declared_.insert(node);
  }

  bool IsDeclared() const {
    return !declared_.empty();
  }

  bool IsType(JSType type) const {
    if (primary_typed_ == TYPE_NOT_SEARCHED ||
        primary_typed_ == TYPE_ANY) {
      return true;
    }
    return primary_typed_ == type;
  }

  JSType GetPrimaryType() const {
    return primary_typed_;
  }

  void ToInitilaize(JSType type) {
    // this type as primary_typed_
    primary_typed_ = type;
    typed_.insert(type);
    is_initialized_ = true;
  }

  void InsertType(JSType type) {
    if (primary_typed_ == TYPE_NOT_SEARCHED) {
      primary_typed_ = type;
    }
    typed_.insert(type);
  }

 private:
  VariableType variable_type_;
  DeclaredSet declared_;
  JSType primary_typed_;
  TypeSet typed_;
  bool is_initialized_;
  std::size_t assigned_count_;
  bool is_referenced_;
};

typedef std::unordered_map<iv::core::UString, Var> VariableMap;

}  // namespace az
#endif  // _AZ_VARIABLE_TYPE_H_
