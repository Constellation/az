#ifndef _AZ_CFA2_HEAP_H_
#define _AZ_CFA2_HEAP_H_
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/cfa2/binding.h>
namespace az {
namespace cfa2 {

class Heap : private iv::core::Noncopyable<Heap> {
 public:
  Binding* Instantiate(Symbol name) {
    heap_.insert(std::shared_ptr<Binding>(new Binding(name, Binding::STACK)));
  }

 private:
  std::unordered_set<std::shared_ptr<Binding> > heap_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_HEAP_H_
