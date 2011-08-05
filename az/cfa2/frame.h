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
    const std::pair<AVal, uint64_t> mod = heap->modified_[binding];
    if (pair.second < mod.second) {
      pair.first.Join(mod.first);
      pair.second = mod.second;
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
