#ifndef AZ_CFA2_FRAME_H_
#define AZ_CFA2_FRAME_H_
#include <iv/detail/unordered_map.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aval.h>
#include <az/cfa2/state.h>
#include <az/cfa2/interpreter_fwd.h>
namespace az {
namespace cfa2 {

class Frame {
 public:
  typedef std::unordered_map<Binding*, std::pair<AVal, State> > Table;

  explicit Frame(Interpreter* interp)
    : interp_(interp),
      prev_(interp->CurrentFrame()),
      table_(),
      this_binding_(),
      rest_() {
    interp_->set_current_frame(this);
  }

  ~Frame() {
    interp_->set_current_frame(prev_);
  }

  void Set(Heap* heap, Binding* binding, const AVal& val) {
    table_[binding] = std::make_pair(val, heap->state());
  }

  void SetThis(const AVal& binding) {
    this_binding_ = binding;
  }

  const AVal& GetThis() const {
    return this_binding_;
  }

  void SetRest(const AVal& rest) {
    rest_ = rest;
  }

  // check heap and update and return
  AVal Get(Heap* heap, Binding* binding) {
    std::pair<AVal, State> pair = table_[binding];
    if (pair.second < binding->state()) {
      pair.first.Join(binding->value());
      pair.second = binding->state();
      table_[binding] = pair;
    }
    return pair.first;
  }

  bool IsDefined(Heap* heap, Binding* binding) const {
    return table_.find(binding) != table_.end();
  }

 private:
  Interpreter* interp_;
  Frame* prev_;
  Table table_;
  AVal this_binding_;
  AVal rest_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_FRAME_H_
