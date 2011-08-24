#include <gtest/gtest.h>
#include <cstring>
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
    const std::array<Token::Type, 1> expected = { {
      Token::TK_CONST
    } };
    std::array<Token::Type, 1>::const_iterator ex = expected.begin();
    for (std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
         tag; tag = parser.Next(), ++ex) {
      EXPECT_EQ(*ex, tag->token());
    }
  }

  {
    const iv::core::UString str =
        iv::core::ToUString("/**@const\n @const*/");
    az::jsdoc::Parser parser(str);
    const std::array<Token::Type, 2> expected = { {
      Token::TK_CONST,
      Token::TK_CONST
    } };
    std::array<Token::Type, 2>::const_iterator ex = expected.begin();
    for (std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
         tag; tag = parser.Next(), ++ex) {
      EXPECT_EQ(*ex, tag->token());
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
    const std::array<Token::Type, 3> expected = { {
      Token::TK_CONST,
      Token::TK_CONST,
      Token::TK_CONST
    } };
    std::array<Token::Type, 3>::const_iterator ex = expected.begin();
    for (std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
         tag; tag = parser.Next(), ++ex) {
      EXPECT_EQ(*ex, tag->token());
    }
  }
}

TEST(JSDocParser, TypeParseTest) {
  using az::jsdoc::Token;
  {
    const iv::core::UString str = iv::core::ToUString(
        "/**\n"
        " * @param {String} userName\n"
        " * @param {String userName\n"
        "*/");
    az::jsdoc::Parser parser(str);
    {
      const std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
      ASSERT_TRUE(tag);
      EXPECT_EQ(Token::TK_PARAM, tag->token());
      EXPECT_TRUE(iv::core::ToUString("String") == tag->type());
    }
    {
      // this tag is failed
      const std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
      EXPECT_FALSE(tag);
    }
  }
}

TEST(JSDocParser, ParamParseTest) {
  using az::jsdoc::Token;
  {
    const iv::core::UString str = iv::core::ToUString(
        "/**\n"
        " * @param {String} userName\n"
        "*/");
    az::jsdoc::Parser parser(str);
    const std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
    EXPECT_EQ(Token::TK_PARAM, tag->token());
    EXPECT_TRUE(iv::core::ToUString("String") == tag->type());
    EXPECT_TRUE(iv::core::ToUString("userName") == tag->name());
  }
}

TEST(JSDocParser, TypeExpressionScanTest) {
  using az::jsdoc::Token;
  {
    const iv::core::UString str = iv::core::ToUString(
        "/**\n"
        " * @param {{ok:String}} userName\n"
        "*/");
    az::jsdoc::Parser parser(str);
    const std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
    EXPECT_EQ(Token::TK_PARAM, tag->token());
    EXPECT_TRUE(iv::core::ToUString("{ok:String}") == tag->type());
    EXPECT_TRUE(iv::core::ToUString("userName") == tag->name());
  }
  {
    const iv::core::UString str = iv::core::ToUString(
        "/**\n"
        " * @param {{ok:String} userName\n"
        "*/");
    az::jsdoc::Parser parser(str);
    const std::shared_ptr<az::jsdoc::Tag> tag = parser.Next();
    EXPECT_FALSE(tag);
  }
}
