#include <gtest/gtest.h>
#include <iv/detail/array.h>
#include <iv/ustring.h>
#include <az/jsdoc/type_lexer.h>

TEST(JSDocTypeLexer, TypeExpressionLexerTest) {
  {
    const iv::core::UString str = iv::core::ToUString("{ok:String}");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LBRACE, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COLON, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RBRACE, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("ok.String");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("ok.String.<String>");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_DOT_LT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_GT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("ok.String.<String|Number>");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_DOT_LT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_PIPE, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_GT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("ok.String.<String|Number>");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_DOT_LT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_PIPE, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_GT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
}

TEST(JSDocTypeLexer, TypeExpressionLexerTest2) {
  // from closure compiler type annotation manual
  {
    const iv::core::UString str = iv::core::ToUString("boolean");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("Window");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("goog.ui.Menu");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("Array.<string>");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_DOT_LT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_GT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("Object.<string, number>");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_DOT_LT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COMMA, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_GT, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("(number|boolean)");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_PIPE, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("{myNum: number, myObject}");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LBRACE, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COLON, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COMMA, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RBRACE, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("?number");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_QUESTION, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("!object");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_BANG, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(string, boolean)");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COMMA, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(): number");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COLON, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(this:goog.ui.Menu, string)");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COLON, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COMMA, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(new:goog.ui.Menu, string)");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COLON, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COMMA, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(string, ...[number])");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COMMA, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_REST, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LBRACK, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RBRACK, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("...number");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_REST, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("number=");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EQUAL, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(?string=, number=)");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_LPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_QUESTION, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EQUAL, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_COMMA, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_NAME, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EQUAL, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_RPAREN, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
  {
    const iv::core::UString str = iv::core::ToUString("*");
    az::jsdoc::TypeLexer lexer(str);
    ASSERT_EQ(az::jsdoc::TypeToken::TK_STAR, lexer.Next());
    ASSERT_EQ(az::jsdoc::TypeToken::TK_EOS, lexer.Next());
  }
}
