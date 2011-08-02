#ifndef _AZ_CFA2_BINDING_H_
#define _AZ_CFA2_BINDING_H_
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/symbol.h>
namespace az {
namespace cfa2 {

// target variable decls
class Binding : private iv::core::Noncopyable<Binding> {
 public:
  enum Type {
    STACK = 0,
    HEAP
  };

  explicit Binding(Symbol name, Type type)
    : name_(name),
      type_(type),
      value_() {
  }

 private:
  Symbol name_;
  Type type_;
  std::shared_ptr<AVal> value_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BINDING_H_
