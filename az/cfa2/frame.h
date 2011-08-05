#ifndef _AZ_CFA2_FRAME_H_
#define _AZ_CFA2_FRAME_H_
#include <iv/detail/unordered_map.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aval.h>
namespace az {
namespace cfa2 {

class Frame {
 public:
  typedef std::unordered_map<Binding*, std::pair<AVal, uint64_t> > Table;

  void Set(Heap* heap, Binding* binding, AVal val) {
    table_[binding] = std::make_pair(val, heap->timestamp());
  }

  void SetThis(AVal binding) {
    this_binding_ = binding;
  }

  // check heap and update and return
  AVal Get(Heap* heap, Binding* binding) {
    std::pair<AVal, uint64_t> pair = table_[binding];
    if (pair.second < binding->timestamp()) {
      pair.first.Join(binding->value());
      pair.second = binding->timestamp();
      table_[binding] = pair;
    }
    return pair.first;
  }

 private:
  Table table_;
  AVal this_binding_;
};


} }  // namespace az::cfa2
#endif  // _AZ_CFA2_FRAME_H_
