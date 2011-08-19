#include <gtest/gtest.h>
#include <iv/detail/array.h>
#include <iv/ustring.h>
#include <az/jsdoc/lexer.h>

TEST(JSDocLexerCase, ConstTest) {
  using az::jsdoc::Token;
  {
    const iv::core::UString str = iv::core::ToUString("/** @const */");
    az::jsdoc::Lexer lexer(str);
    const std::array<Token::Type, 2> expected = { {
      Token::TK_CONST,
      Token::TK_EOS
    } };
    std::array<Token::Type, 2>::const_iterator ex = expected.begin();
    for (Token::Type token = lexer.Next();
         token != Token::TK_EOS;
         token = lexer.Next(), ++ex) {
      ASSERT_EQ(*ex, token);
    }
  }
  {
    const iv::core::UString str =
        iv::core::ToUString("/**@const @const*/");
    az::jsdoc::Lexer lexer(str);
    const std::array<Token::Type, 3> expected = { {
      Token::TK_CONST,
      Token::TK_CONST,
      Token::TK_EOS
    } };
    std::array<Token::Type, 3>::const_iterator ex = expected.begin();
    for (Token::Type token = lexer.Next();
         token != Token::TK_EOS;
         token = lexer.Next(), ++ex) {
      ASSERT_EQ(*ex, token);
    }
  }
}
