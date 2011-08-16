#include <gtest/gtest.h>
#include <iv/ustring.h>
#include <az/complete_lexer.h>

TEST(CompleteLexerCase, BuildTest) {
  const iv::core::UString value = iv::core::ToUString("var i = 20;");
  az::CompleteLexer lexer(value, 2);
}
