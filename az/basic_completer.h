#ifndef AZ_BASIC_COMPLETER_H_
#define AZ_BASIC_COMPLETER_H_
#include <az/cfa2/aval_fwd.h>
#include <az/ast_fwd.h>
namespace az {

class BasicCompleter {
 public:
  BasicCompleter()
    : has_completion_point_(false),
      base_(NULL),
      function_(NULL) {
  }

  virtual ~BasicCompleter() { }

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

 private:
  void SetCompletionPoint() {
    has_completion_point_ = true;
  }

  bool has_completion_point_;
  Expression* base_;
  FunctionLiteral* function_;
};

}  // namespace az
#endif  // AZ_COMPLETER_H_
