#ifndef AZ_CFA2_DEBUG_COMPLETER_H_
#define AZ_CFA2_DEBUG_COMPLETER_H_
#include <az/ast_fwd.h>
#include <az/cfa2/completer.h>
namespace az {
namespace cfa2 {

class DebugCompleter : public Completer {
 public:
  typedef std::unordered_map<Symbol, AVal> Properties;

  DebugCompleter()
    : Completer(),
      properties_() {
  }

  void Output() const {
    assert(heap());
  }


  void Notify(Symbol name, const AVal& target) {
    if (properties_.find(name) == properties_.end()) {
      properties_.insert(std::make_pair(name, target));
    } else {
      properties_[name].Join(target);
    }
  }

 private:
  Heap* heap_;
  Properties properties_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_DEBUG_COMPLETER_H_
