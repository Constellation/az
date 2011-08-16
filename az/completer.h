// mark complete tokens
#ifndef _AZ_COMPLETER_H_
#define _AZ_COMPLETER_H_
#include <az/ast_fwd.h>
namespace az {

class Completer {
 public:
  Completer()
    : has_completion_point_(false),
      base_(NULL) {
  }

  bool HasCompletionPoint() const {
    return has_completion_point_;
  }

  void RegisterPropertyCompletion(Expression* base) {
    base_ = base;
    SetCompletionPoint();
  }

 private:
  void SetCompletionPoint() {
    has_completion_point_ = true;
  }

  bool has_completion_point_;
  Expression* base_;
};

}  // namespace az
#endif  // _AZ_COMPLETER_H_
