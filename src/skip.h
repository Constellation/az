#ifndef _AZ_SKIP_H_
#define _AZ_SKIP_H_
#include <iv/keyword.h>
#include "character.h"
namespace az {

// Skip class skips heuristic skip sources
template<typename Lexer>
class BasicSkip {
 public:
  explicit BasicSkip(Lexer* lexer, bool strict)
    : lexer_(lexer),
      strict_(strict) {
  }

  iv::core::Token::Type SkipUntil(iv::core::Token::Type last) {
    using iv::core::Token;
    // skip until this token or semicolon or LineTerminator
    while (true) {
      const Token::Type token = lexer_->template Next<iv::core::IdentifyReservedWords>(strict_);
      if (token == Token::TK_EOS ||
          token == Token::TK_SEMICOLON ||
          token == last) {
        return lexer_->template Next<iv::core::IdentifyReservedWords>(strict_);
      } else {
        // see LineTerminator
        if (lexer_->has_line_terminator_before_next()) {
          // LineTerminator found
          return token;
        }
      }
    }
  }

  iv::core::Token::Type SkipUntilSemicolonOrLineTerminator() {
    using iv::core::Token;
    return SkipUntil(Token::TK_NUM_TOKENS);  // TK_NUM_TOKENS is not match
  }

 private:
  Lexer* lexer_;
  bool strict_;
};

}  // namespace az
#endif  // _AZ_SKIP_H_
