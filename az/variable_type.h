#ifndef AZ_VARIABLE_TYPE_H_
#define AZ_VARIABLE_TYPE_H_
#include <iv/detail/unordered_map.h>
#include <iv/detail/unordered_set.h>
#include <iv/detail/memory.h>
#include <iv/ustring.h>
#include <az/symbol.h>
#include <az/jstype.h>
#include <az/ast_fwd.h>
namespace az {

typedef std::unordered_set<const AstNode*> DeclaredSet;

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

  AType GetType() const {
    return typed_;
  }

  void InsertType(AType type) {
    typed_ = AType::Merged(typed_, type);
  }

 private:
  VariableType variable_type_;
  DeclaredSet declared_;
  AType typed_;
  bool is_initialized_;
  std::size_t assigned_count_;
  bool is_referenced_;
};

typedef std::unordered_map<Symbol, Var> VariableMap;

}  // namespace az
#endif  // AZ_VARIABLE_TYPE_H_
