#include <gtest/gtest.h>
#include <iv/detail/array.h>
#include <iv/ustring.h>
#include <az/jsdoc/parser.h>

TEST(JSDocParserCase, UnwrapCommentTest) {
  using az::jsdoc::Token;
  {
    const iv::core::UString str = iv::core::ToUString(
        "/**\n"
        " * @const\n"
        " * @const\n"
        " */");
    std::string result;
    az::jsdoc::Parser::UnwrapComment(str, std::back_inserter(result));
    EXPECT_EQ("\n"
              "@const\n"
              "@const\n", result);
  }
  {
    const iv::core::UString str = iv::core::ToUString("/**x*/");
    std::string result;
    az::jsdoc::Parser::UnwrapComment(str, std::back_inserter(result));
    EXPECT_EQ("x", result);
  }
  {
    const iv::core::UString str = iv::core::ToUString("/***x*/");
    std::string result;
    az::jsdoc::Parser::UnwrapComment(str, std::back_inserter(result));
    EXPECT_EQ("x", result);
  }
  {
    const iv::core::UString str = iv::core::ToUString("/****x*/");
    std::string result;
    az::jsdoc::Parser::UnwrapComment(str, std::back_inserter(result));
    EXPECT_EQ("*x", result);
  }
  {
    const iv::core::UString str = iv::core::ToUString("/**x\n * y\n*/");
    std::string result;
    az::jsdoc::Parser::UnwrapComment(str, std::back_inserter(result));
    EXPECT_EQ("x\ny\n", result);
  }
  {
    const iv::core::UString str = iv::core::ToUString("/**x\n *   y\n*/");
    std::string result;
    az::jsdoc::Parser::UnwrapComment(str, std::back_inserter(result));
    EXPECT_EQ("x\n  y\n", result);
  }
}

TEST(JSDocParser, TagParseTest) {
  using az::jsdoc::Token;
  {
    const iv::core::UString str = iv::core::ToUString("/** @const */");
    az::jsdoc::Parser parser(str);
    const std::array<Token::Type, 2> expected = { {
      Token::TK_CONST,
      Token::TK_EOS
    } };
    std::array<Token::Type, 2>::const_iterator ex = expected.begin();
    for (Token::Type token = parser.Next();
         token != Token::TK_EOS;
         token = parser.Next(), ++ex) {
      EXPECT_EQ(*ex, token);
    }
  }

  {
    const iv::core::UString str =
        iv::core::ToUString("/**@const\n @const*/");
    az::jsdoc::Parser parser(str);
    const std::array<Token::Type, 3> expected = { {
      Token::TK_CONST,
      Token::TK_CONST,
      Token::TK_EOS
    } };
    std::array<Token::Type, 3>::const_iterator ex = expected.begin();
    for (Token::Type token = parser.Next();
         token != Token::TK_EOS;
         token = parser.Next(), ++ex) {
      EXPECT_EQ(*ex, token);
    }
  }

  {
    const iv::core::UString str =
        iv::core::ToUString(
            "/**\n"
            " * @const @const\n"
            " * @const @const\n"
            " * @const @const\n"
            " */");
    az::jsdoc::Parser parser(str);
    const std::array<Token::Type, 4> expected = { {
      Token::TK_CONST,
      Token::TK_CONST,
      Token::TK_CONST,
      Token::TK_EOS
    } };
    std::array<Token::Type, 3>::const_iterator ex = expected.begin();
    for (Token::Type token = parser.Next();
         token != Token::TK_EOS;
         token = parser.Next(), ++ex) {
      EXPECT_EQ(*ex, token);
    }
  }
}
