#ifndef _AZ_CFA2_HEAP_H_
#define _AZ_CFA2_HEAP_H_
#include <algorithm>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/cfa2/binding.h>
#include <az/deleter.h>
namespace az {
namespace cfa2 {

class Heap : private iv::core::Noncopyable<Heap> {
 public:
  typedef std::unordered_set<Binding*> HeapSet;
  Heap()
    : heap_(),
      declared_heap_bindings_() {
  }

  ~Heap() {
    std::for_each(heap_.begin(), heap_.end(), Deleter());
  }

  // create new binding object
  Binding* Instantiate(Symbol name) {
    Binding* binding = new Binding(name, Binding::STACK);
    heap_.insert(binding);
    return binding;
  }

  void RecordDeclaredHeapBinding(Binding* binding) {
    declared_heap_bindings_.insert(binding);
  }

 private:
  HeapSet heap_;
  HeapSet declared_heap_bindings_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_HEAP_H_
