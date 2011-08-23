#ifndef _AZ_JSDOC_PROVIDER_H_
#define _AZ_JSDOC_PROVIDER_H_
#include <iv/ustringpiece.h>
#include <az/jsdoc/info.h>
#include <az/jsdoc/parser.h>
namespace az {
namespace jsdoc {

class Provider {
 public:
  Provider()
    : info_() {
  }

  void Parse(const iv::core::UStringPiece& src) {
    Parser parser(src);
    for (std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
         tag; tag = parser.Next()) {
      info_.Register(tag);
    }
  }

  const Info& GetInfo() const {
    return info_;
  }
 private:
  Info info_;
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_PROVIDER_H_
