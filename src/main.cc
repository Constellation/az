#include <cstdio>
#include <string>
#include <iv/parser.h>
#include "factory.h"

int main(int argc, char** argv) {
  typedef iv::core::Parser<az::AstFactory, std::string, true, false> Parser;
  const std::string src("var i = 20;");
  az::AstFactory factory;
  Parser parser(&factory, src);
  const az::FunctionLiteral* const global = parser.ParseProgram();
  if (!global) {
    // syntax error occurred
    std::fprintf(stderr, "%s\n", parser.error().c_str());
  } else {
  }
  return 0;
}
