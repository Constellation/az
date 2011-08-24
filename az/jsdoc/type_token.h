#ifndef _AZ_JSDOC_TYPE_TOKEN_H_
#define _AZ_JSDOC_TYPE_TOKEN_H_
namespace az {
namespace jsdoc {
class TypeToken {
 public:
  enum Type {
    TK_ILLEGAL,    // ILLEGAL
    TK_DOT,        // .
    TK_DOT_LT,     // .<
    TK_REST,       // ...
    TK_GT,         // >
    TK_LPAREN,     // (
    TK_RPAREN,     // )
    TK_LBRACE,     // {
    TK_RBRACE,     // }
    TK_LBRACK,     // [
    TK_RBRACK,     // ]
    TK_COMMA,      // ,
    TK_COLON,      // :
    TK_STAR,       // *
    TK_PIPE,       // |
    TK_QUESTION,   // ?
    TK_BANG,       // !
    TK_EQUAL,      // =
    TK_NAME,       // name token
    TK_EOS
  };
};
} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TYPE_TOKEN_H_
