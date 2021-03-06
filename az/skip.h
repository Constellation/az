#ifndef AZ_SKIP_H_
#define AZ_SKIP_H_
#include <iv/keyword.h>
#include <az/character.h>
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
    const Token::Type start = lexer_->Peek();
    // first check
    if (start == last) {
      // not consume it
      return start;
    } else if (start == Token::TK_EOS ||
               start == Token::TK_SEMICOLON) {
      return lexer_->template Next<iv::core::IdentifyReservedWords>(strict_);
    } else {
      // see LineTerminator (not ILLEGAL)
      if (start != Token::TK_ILLEGAL &&
          lexer_->has_line_terminator_before_next()) {
        // LineTerminator found
        return start;
      }
    }

    // skip until this token or semicolon or LineTerminator
    while (true) {
      const Token::Type token =
          lexer_->template Next<iv::core::IdentifyReservedWords>(strict_);
      if (token == last) {
        // not consume it
        return token;
      } else if (token == Token::TK_EOS ||
                 token == Token::TK_SEMICOLON) {
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
    return SkipUntil(Token::TK_NUM_TOKENS);  // TK_NUM_TOKENS is never match
  }

 private:
  Lexer* lexer_;
  bool strict_;
};

}  // namespace az
#endif  // AZ_SKIP_H_
