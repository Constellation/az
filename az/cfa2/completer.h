// mark complete tokens
#ifndef _AZ_CFA2_COMPLETER_H_
#define _AZ_CFA2_COMPLETER_H_
#include <az/ast_fwd.h>
#include <az/basic_completer.h>
namespace az {
namespace cfa2 {

class Completer : public BasicCompleter {
 public:
  virtual void Notify(Symbol name, const AVal& target) = 0;

  void set_heap(Heap* heap) {
    heap_ = heap;
  }

  Heap* heap() const {
    return heap_;
  }

 private:
  Heap* heap_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_COMPLETER_H_
