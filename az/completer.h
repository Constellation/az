// mark complete tokens
#ifndef _AZ_COMPLETER_H_
#define _AZ_COMPLETER_H_
#include <az/ast_fwd.h>
namespace az {

class Completer {
 public:
  Completer()
    : has_completion_point_(false),
      base_(NULL),
      function_(NULL),
      already_() {
  }

  virtual ~Completer() { }

  bool HasCompletionPoint() const {
    return has_completion_point_;
  }

  bool HasTargetFunction() const {
    return function_;
  }

  void RegisterPropertyCompletion(Expression* base) {
    base_ = base;
    SetCompletionPoint();
  }

  FunctionLiteral* GetTargetFunction() const {
    return function_;
  }

  Expression* GetTargetExpression() const {
    return base_;
  }

  // register target function that has completion target expr node
  void RegisterTargetFunction(FunctionLiteral* function) {
    assert(HasCompletionPoint() && !HasTargetFunction());
    function_ = function;
  }

  virtual void Notify(Symbol name) {
    if (already_.find(name) == already_.end()) {
      already_.insert(name);
      iv::core::UString target = GetSymbolString(name);
      iv::core::unicode::FPutsUTF16(stdout, target.begin(), target.end());
      std::cout << std::endl;
    }
  }

 private:
  void SetCompletionPoint() {
    has_completion_point_ = true;
  }

  bool has_completion_point_;
  Expression* base_;
  FunctionLiteral* function_;
  std::unordered_set<Symbol> already_;
};

}  // namespace az
#endif  // _AZ_COMPLETER_H_
