#ifndef _AZ_NPAPI_JSON_COMPLETER_H_
#define _AZ_NPAPI_JSON_COMPLETER_H_
#include <vector>
#include <string>
#include <az/ast_fwd.h>
#include <az/cfa2/completer.h>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
namespace az {
namespace npapi {

class JSONCompleter : public cfa2::Completer {
 public:
  void Notify(Symbol name, const cfa2::AVal& target) {
    if (names_.find(name) == names_.end()) {
      names_.insert(name);
    }
  }

  std::string Output() const {
    std::stringstream ss;
    boost::property_tree::ptree root;
    std::size_t index = 0;
    for (std::unordered_set<Symbol>::const_iterator it = names_.begin(),
         last = names_.end(); it != last; ++it, ++index) {
      const std::string target = boost::lexical_cast<std::string>(index);
      root.add(target, ToString(*it));
    }
    boost::property_tree::json_parser::write_json(ss, root);
    return ss.str();
  }

  static std::string ToString(Symbol name) {
    std::string result;
    const iv::core::UString prop = GetSymbolString(name);
    result.reserve(prop.size());
    iv::core::unicode::UTF16ToUTF8(prop.begin(), prop.end(),
                                   std::back_inserter(result));
    return result;
  }

 private:
  std::unordered_set<Symbol> names_;
};

} }  // namespace az::npapi
#endif  // _AZ_NPAPI_JSON_COMPLETER_H_
