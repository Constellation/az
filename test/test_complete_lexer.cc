#include <gtest/gtest.h>
#include <iv/detail/array.h>
#include <iv/ustring.h>
#include <az/complete_lexer.h>

TEST(CompleteLexerCase, BuildTest) {
  using iv::core::Token;
  {
    // completion point is
    // test.
    //      ^
    const iv::core::UString value = iv::core::ToUString("test. ");
    az::CompleteLexer lexer(value, 6);
    const std::array<Token::Type, 3> expected = { {
      Token::TK_IDENTIFIER,
      Token::TK_PERIOD,
      Token::TK_ILLEGAL
    } };
    std::array<Token::Type, 3>::const_iterator ex = expected.begin();
    for (Token::Type token = lexer.Next<iv::core::IdentifyReservedWords>(false);
         token != Token::TK_EOS;
         token = lexer.Next<iv::core::IdentifyReservedWords>(false), ++ex) {
      ASSERT_EQ(*ex, token);
    }
    ASSERT_TRUE(lexer.IsCompletionPoint());
    ASSERT_EQ(Token::TK_EOS, lexer.Next<iv::core::IdentifyReservedWords>(false));
  }
}
