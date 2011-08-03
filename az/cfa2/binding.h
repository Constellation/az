#ifndef _AZ_CFA2_BINDING_H_
#define _AZ_CFA2_BINDING_H_
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/symbol.h>
namespace az {
namespace cfa2 {

class AVal;

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
      value_() {
  }

  Symbol name() const {
    return name_;
  }

  Type type() const {
    return type_;
  }

  std::shared_ptr<AVal> value() const {
    return value_;
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
  std::shared_ptr<AVal> value_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BINDING_H_
