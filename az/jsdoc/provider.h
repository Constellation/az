#ifndef AZ_JSDOC_PROVIDER_H_
#define AZ_JSDOC_PROVIDER_H_
#include <iv/ustringpiece.h>
#include <az/jsdoc/info.h>
#include <az/jsdoc/parser.h>
namespace az {
namespace jsdoc {

class Provider {
 public:
  Provider(AstFactory* factory)
    : factory_(factory),
      info_(new Info()) {
  }

  void Parse(const iv::core::UStringPiece& src) {
    Parser parser(factory_, src);
    for (std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
         tag; tag = parser.Next()) {
      info_->Register(tag);
    }
  }

  std::shared_ptr<Info> GetInfo() {
    return info_;
  }
 private:
  AstFactory* factory_;
  std::shared_ptr<Info> info_;
};

} }  // namespace az::jsdoc
#endif  // AZ_JSDOC_PROVIDER_H_
