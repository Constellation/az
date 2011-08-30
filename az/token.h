#ifndef AZ_TOKEN_H_
#define AZ_TOKEN_H_
#include <iv/token.h>
namespace az {

inline bool IsExpressionStartToken(iv::core::Token::Type token) {
  using iv::core::Token;
  return
      // literals and identifiers
      token == Token::TK_STRING ||
      token == Token::TK_NUMBER ||
      token == Token::TK_DIV ||
      token == Token::TK_ASSIGN_DIV ||
      token == Token::TK_TRUE_LITERAL ||
      token == Token::TK_FALSE_LITERAL ||
      token == Token::TK_NULL_LITERAL ||
      token == Token::TK_THIS ||
      token == Token::TK_FUNCTION ||
      token == Token::TK_IDENTIFIER ||
      // new
      token == Token::TK_NEW ||
      // unary operations
      token == Token::TK_DELETE ||
      token == Token::TK_VOID ||
      token == Token::TK_TYPEOF ||
      token == Token::TK_INC ||
      token == Token::TK_DEC ||
      token == Token::TK_ADD ||
      token == Token::TK_SUB ||
      token == Token::TK_NOT ||
      token == Token::TK_BIT_NOT ||
      // ( { [
      token == Token::TK_LPAREN ||
      token == Token::TK_LBRACE ||
      token == Token::TK_LBRACK;
}

inline bool IsStatementStartToken(iv::core::Token::Type token) {
  using iv::core::Token;
  return
      token == Token::TK_LBRACE ||
      token == Token::TK_FUNCTION ||
      token == Token::TK_VAR ||
      token == Token::TK_CONST ||
      token == Token::TK_SEMICOLON ||
      IsExpressionStartToken(token) ||
      token == Token::TK_IF ||
      token == Token::TK_FOR ||
      token == Token::TK_WHILE ||
      token == Token::TK_DO ||
      token == Token::TK_CONTINUE ||
      token == Token::TK_BREAK ||
      token == Token::TK_RETURN ||
      token == Token::TK_WITH ||
      token == Token::TK_IDENTIFIER ||
      token == Token::TK_SWITCH ||
      token == Token::TK_THROW ||
      token == Token::TK_TRY ||
      token == Token::TK_DEBUGGER;
}

}  // namespace az
#endif  // AZ_TOKEN_H_
