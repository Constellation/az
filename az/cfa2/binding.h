#ifndef _AZ_CFA2_BINDING_H_
#define _AZ_CFA2_BINDING_H_
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/symbol.h>
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/state.h>
namespace az {
namespace cfa2 {

// target variable decls
class Binding : private iv::core::Noncopyable<Binding> {
 public:
  enum Type {
    STACK = 0,
    HEAP,
    GLOBAL
  };

  explicit Binding(Symbol name, Type type)
    : name_(name),
      type_(type),
      state_(kInvalidState),
      value_(AVAL_NOBASE) {
  }

  Symbol name() const {
    return name_;
  }

  Type type() const {
    return type_;
  }

  State state() const {
    return state_;
  }

  void set_state(State state) {
    state_ = state;
  }

  AVal value() const {
    return value_;
  }

  void set_value(const AVal& val) {
    value_ = val;
  }

  void ToHeap() {
    type_ = HEAP;
  }

  void ToStack() {
    type_ = STACK;
  }

 private:
  Symbol name_;
  Type type_;
  State state_;
  AVal value_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BINDING_H_
