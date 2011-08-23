#ifndef _AZ_JSDOC_INFO_H_
#define _AZ_JSDOC_INFO_H_
#include <iv/ustringpiece.h>
namespace az {
namespace jsdoc {

class Info {
 public:
  explicit Info(Token::Type token)
    : token_ (token),
      type_(),
      name_(),
      description_() {
  }

  Info(Token::Type token, const iv::core::UStringPiece& desc)
    : token_(token),
      type_(),
      name_(),
      description_(desc) {
  }

  Token::Type token() const {
    return token_;
  }

  const iv::core::UString& type() const {
    return type_;
  }

  const iv::core::UString& name() const {
    return name_;
  }

  const iv::core::UString& description() const {
    return description_;
  }

 private:
  Token::Type token_;
  iv::core::UString type_;
  iv::core::UString name_;
  iv::core::UString description_;
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_INFO_H_
