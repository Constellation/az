// mark complete tokens
#ifndef _AZ_CFA2_CLI_COMPLETER_H_
#define _AZ_CFA2_CLI_COMPLETER_H_
#include <az/ast_fwd.h>
#include <az/cfa2/completer.h>
namespace az {
namespace cfa2 {

class CLICompleter : public Completer {
 public:
  typedef std::unordered_map<Symbol, AVal> Properties;

  CLICompleter()
    : Completer(),
      properties_() {
  }

  void Output() const {
    assert(heap());
    for (Properties::const_iterator it = properties_.begin(),
         last = properties_.end(); it != last; ++it) {
      const iv::core::UString target = GetSymbolString(it->first);
      const iv::core::UString type = it->second.ToTypeString(heap());
      iv::core::unicode::FPutsUTF16(stdout, target.begin(), target.end());
      std::fputc('#', stdout);
      iv::core::unicode::FPutsUTF16(stdout, type.begin(), type.end());
      std::fputc('\n', stdout);
    }
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
#endif  // _AZ_CFA2_CLI_COMPLETER_H_
