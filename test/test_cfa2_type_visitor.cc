#include <gtest/gtest.h>
#include <az/factory.h>
#include <az/jsdoc/type_parser.h>
#include <az/cfa2/type_ast_visitor.h>

TEST(CFA2TypeAstVisitor, TypeAstVisitorTest) {
  az::AstFactory factory;
  {
    const iv::core::UString str = iv::core::ToUString("{ok:String}");
    az::jsdoc::TypeParser parser(&factory, str);
    az::jsdoc::TypeExpression* expr = parser.ParseParamType();
    ASSERT_TRUE(expr);
    az::cfa2::TypeAstVisitor visitor;
    expr->Accept(&visitor);
  }
}
