#include <iv/ustring.h>
#include <iv/unicode.h>
#include <az/factory.h>
#include <az/analyzer.h>
#include <az/reporter.h>
#include <az/parser.h>
#include <az/symbol.h>
#include "analyze.h"
#include "json_reporter.h"
#include "utils.h"
#include "debug.h"
namespace az {

bool Analyze(NPNetscapeFuncs* np,
             NPObject* receiver, const iv::core::StringPiece& piece, NPVariant* result) {
  iv::core::UString src;
  src.reserve(piece.size());
  if (iv::core::unicode::UTF8ToUTF16(
          piece.begin(),
          piece.end(),
          std::back_inserter(src)) != iv::core::unicode::NO_ERROR) {
    np->setexception(receiver, "invalid UTF-8 encoding text");
    BOOLEAN_TO_NPVARIANT(false, *result);
    return false;
  }
  StructuredSource structured(src);
  JSONReporter reporter(structured);
  AstFactory factory;
  Parser<iv::core::UString, JSONReporter> parser(&factory, src, &reporter, structured);
  FunctionLiteral* const global = parser.ParseProgram();
  if (!global) {
    // syntax error occurred
    std::fprintf(stderr, "%s\n", parser.errors().back().c_str());
  } else {
    // az::Analyze(global, src, &reporter);
  }
  StringToNPVariant(np, reporter.Output(), result);
  return true;
}

}  // namespace az
