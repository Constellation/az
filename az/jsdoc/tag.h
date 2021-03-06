#ifndef AZ_JSDOC_TAG_H_
#define AZ_JSDOC_TAG_H_
#include <iv/stringpiece.h>
#include <iv/ustringpiece.h>
#include <az/jsdoc/fwd.h>
#include <az/jsdoc/token.h>
#include <az/jsdoc/type_ast_fwd.h>
namespace az {
namespace jsdoc {

static const TagArray kKnownTags = { {
  "@const",
  "@constructor",
  "@define",
  "@deprecated",
  "@enum",
  "@extends",
  "@implements",
  "@inheritDoc",
  "@interface",
  "@license",
  "@preserve",
  "@nosideeffects",
  "@override",
  "@param",
  "@private",
  "@protected",
  "@return",
  "@this",
  "@type",
  "@typedef"
} };

class Tag {
 public:
  friend class Parser;
  explicit Tag(Token::Type token)
    : token_ (token),
      type_(),
      name_(),
      description_() {
  }

  Token::Type token() const {
    return token_;
  }

  TypeExpression* type() const {
    return type_;
  }

  const iv::core::UString& name() const {
    return name_;
  }

  const iv::core::UString& description() const {
    return description_;
  }

 private:
  void set_title(const std::vector<uint16_t>& vec) {
    title_.assign(vec.begin(), vec.end());
  }

  void set_type(TypeExpression* expr) {
    type_ = expr;
  }

  void set_name(const std::vector<uint16_t>& vec) {
    name_.assign(vec.begin(), vec.end());
  }

  void set_description(const std::vector<uint16_t>& vec) {
    description_.assign(vec.begin(), vec.end());
  }

  Token::Type token_;
  iv::core::UString title_;
  TypeExpression* type_;
  iv::core::UString name_;
  iv::core::UString description_;
};

} }  // namespace az::jsdoc
#endif  // AZ_JSDOC_TAG_H_
