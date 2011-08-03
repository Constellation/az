#ifndef _AZ_CFA2_HEAP_H_
#define _AZ_CFA2_HEAP_H_
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/cfa2/binding.h>
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
    for (HeapSet::const_iterator it = heap_.begin(),
         last = heap_.end(); it != last; ++it) {
      delete *it;
    }
  }

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
