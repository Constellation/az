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

 private:
  HeapSet heap_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_HEAP_H_
