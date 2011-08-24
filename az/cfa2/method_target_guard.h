#ifndef _AZ_CFA2_METHOD_TARGET_GUARD_H_
#define _AZ_CFA2_METHOD_TARGET_GUARD_H_
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <az/cfa2/heap.h>
#include <az/cfa2/interpreter_fwd.h>
namespace az {
namespace cfa2 {

class MethodTargetGuard : private iv::core::Noncopyable<MethodTargetGuard> {
 public:
  MethodTargetGuard(Interpreter* interp, Expression* expr)
    : interp_(interp),
      expr_(expr) {
  }

  ~MethodTargetGuard() {
    if (interp_->heap()->IsMethodTarget(expr_)) {
      interp_->heap()->MergeMethodTarget(expr_,
                                         interp_->result().result());
    }
  }

 private:
  Interpreter* interp_;
  Expression* expr_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_METHOD_TARGET_GUARD_H_
