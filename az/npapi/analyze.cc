#include <iv/detail/cstdint.h>
#include <iv/detail/cinttypes.h>
#include <iv/ustring.h>
#include <iv/unicode.h>
#include <az/factory.h>
#include <az/analyzer.h>
#include <az/reporter.h>
#include <az/empty_reporter.h>
#include <az/parser.h>
#include <az/symbol.h>
#include <az/basic_completer.h>
#include <az/complete_lexer.h>
#include <az/cfa2.h>
#include <az/npapi/analyze.h>
#include <az/npapi/json_reporter.h>
#include <az/npapi/json_completer.h>
#include <az/npapi/utils.h>
#include <az/npapi/debug.h>
namespace az {
namespace npapi {

bool Analyze(NPNetscapeFuncs* np,
             NPObject* receiver, const iv::core::StringPiece& piece, NPVariant* result) {
  typedef az::Parser<iv::core::UString,
                     CompleteLexer,
                     JSONReporter,
                     BasicCompleter> Parser;
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
  Context ctx;
  StructuredSource structured(src);
  JSONReporter reporter(structured);
  AstFactory factory;
  CompleteLexer lexer(src);
  Parser parser(&ctx, &factory, src, &lexer, &reporter, NULL, structured);
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

bool Complete(NPNetscapeFuncs* np,
              NPObject* receiver,
              const iv::core::StringPiece& piece,
              std::size_t len,
              NPVariant* result) {
  typedef az::Parser<iv::core::UString,
                     CompleteLexer,
                     EmptyReporter,
                     BasicCompleter> Parser;
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
  if (len > src.size()) {
    std::fprintf(stderr,
                 "%s " "%" PRIu64 " %s\n",
                 "pulse position",
                 static_cast<uint64_t>(len),
                 "is out of range");
    np->setexception(receiver, "invaid completion position");
    return false;
  }
  Context ctx;
  StructuredSource structured(src);
  EmptyReporter reporter;
  AstFactory factory;
  JSONCompleter completer;
  CompleteLexer lexer(src, len);
  Parser parser(&ctx, &factory, src, &lexer, &reporter, &completer, structured);
  FunctionLiteral* const global = parser.ParseProgram();
  assert(global);
  if (completer.HasCompletionPoint()) {
    cfa2::Heap heap(&factory, &completer);
    cfa2::Complete(global, &heap, src, &reporter);
    StringToNPVariant(np, completer.Output(), result);
  } else {
    StringToNPVariant(np, "null", result);
  }
  return true;
}

} }  // namespace az::npapi
