#include <iostream>
#include <gtest/gtest.h>
#include <iv/detail/array.h>
#include <iv/ustring.h>
#include <az/factory.h>
#include <az/jsdoc/type_ast.h>
#include <az/jsdoc/type_parser.h>

TEST(JSDocTypeParser, TypeExpressionParserTest) {
  az::AstFactory factory;
  {
    const iv::core::UString str = iv::core::ToUString("{ok:String}");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsRecordType());
  }
  {
    const iv::core::UString str = iv::core::ToUString("ok.String");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsNameExpression());
  }
  {
    const iv::core::UString str = iv::core::ToUString("ok.String.<String>");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsTypeNameWithApplication());
  }
  {
    // failed case
//    const iv::core::UString str = iv::core::ToUString("ok.String.<String|Number>");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsTypeNameWithApplication());
  }
  {
//    const iv::core::UString str = iv::core::ToUString("ok.String.<String|Number>");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsTypeNameWithApplication());
  }
}


TEST(JSDocTypeParser, TypeExpressionParserTest2) {
  // from closure compiler type annotation manual
  az::AstFactory factory;
  {
    const iv::core::UString str = iv::core::ToUString("boolean");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsNameExpression());
  }
  {
    const iv::core::UString str = iv::core::ToUString("Window");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsNameExpression());
  }
  {
    const iv::core::UString str = iv::core::ToUString("goog.ui.Menu");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsNameExpression());
  }
  {
    const iv::core::UString str = iv::core::ToUString("Array.<string>");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsTypeNameWithApplication());
  }
//  {
//    const iv::core::UString str = iv::core::ToUString("Object.<string, number>");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsTypeNameWithApplication());
//  }
  {
//    const iv::core::UString str = iv::core::ToUString("{(number|boolean)}");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsRecordType());
  }
  {
    const iv::core::UString str = iv::core::ToUString("{myNum: number, myObject}");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsRecordType());
  }
  {
    const iv::core::UString str = iv::core::ToUString("?number");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsPrefixQuestionExpression());
  }
  {
    const iv::core::UString str = iv::core::ToUString("!object");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsPrefixBangExpression());
  }
  {
//    const iv::core::UString str = iv::core::ToUString("function(string, boolean)");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsFunctionType());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(): number");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsFunctionType());
  }
  {
//    const iv::core::UString str = iv::core::ToUString("function(this:goog.ui.Menu, string)");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsFunctionType());
  }
  {
//    const iv::core::UString str = iv::core::ToUString("function(new:goog.ui.Menu, string)");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsFunctionType());
  }
  {
    const iv::core::UString str = iv::core::ToUString("function(string, ...[number])");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsFunctionType());
  }
  {
//    const iv::core::UString str = iv::core::ToUString("...number");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsRestExpression());
  }
  {
//    const iv::core::UString str = iv::core::ToUString("number=");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsPostfixEqualExpression());
  }
  {
//    const iv::core::UString str = iv::core::ToUString("function(?string=, number=)");
//    az::jsdoc::TypeParser parser(&factory, str);
//    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
//    ASSERT_TRUE(expr);
//    EXPECT_TRUE(expr->AsFunctionType());
  }
  {
    const iv::core::UString str = iv::core::ToUString("*");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    EXPECT_TRUE(expr->AsStarLiteral());
  }
}
