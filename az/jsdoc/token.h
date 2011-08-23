#ifndef _AZ_JSDOC_TOKEN_H_
#define _AZ_JSDOC_TOKEN_H_
namespace az {
namespace jsdoc {

class Token {
 public:
  enum Type {
    TK_CONST = 0,
    TK_CONSTRUCTOR,
    TK_DEFINE,
    TK_DEPRECATED,
    TK_ENUM,
    TK_EXTENDS,
    TK_IMPLEMENTS,
    TK_INHERITDOC,
    TK_INTERFACE,
    TK_LICENSE,
    TK_PRESERVE,
    TK_NOSIDEEFFECTS,
    TK_OVERRIDE,
    TK_PARAM,
    TK_PRIVATE,
    TK_PROTECTED,
    TK_RETURN,
    TK_THIS,
    TK_TYPE,
    TK_TYPEDEF,
    TK_TAG,  // not known tags
    TK_EOS,
    TK_NOT_FOUND
  };
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TOKEN_H_
