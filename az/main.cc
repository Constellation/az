#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iterator>
#include <iv/detail/array.h>
#include <iv/parser.h>
#include <iv/ustring.h>
#include <iv/unicode.h>
#include <iv/cmdline.h>
#include <iv/debug.h>
#include <az/factory.h>
#include <az/analyzer.h>
#include <az/reporter.h>
#include <az/empty_reporter.h>
#include <az/basic_completer.h>
#include <az/parser.h>
#include <az/context.h>
#include <az/symbol.h>
#include <az/debug_log.h>
#include <az/cfa2.h>
namespace {

bool ReadFile(const std::string& filename, std::vector<char>* out) {
  if (std::FILE* fp = std::fopen(filename.c_str(), "r")) {
    std::array<char, 1024> buf;
    while (const std::size_t len = std::fread(
            buf.data(),
            1,
            buf.size(), fp)) {
      out->insert(out->end(), buf.begin(), buf.begin() + len);
    }
    std::fclose(fp);
    return true;
  } else {
    std::string err("az can't open \"");
    err.append(filename);
    err.append("\"");
    std::perror(err.c_str());
    return false;
  }
}

inline int Pulse(const iv::core::UString& src, std::size_t len) {
  typedef az::Parser<iv::core::UString,
                     az::CompleteLexer,
                     az::EmptyReporter,
                     az::BasicCompleter> Parser;
  az::Context ctx;
  az::StructuredSource structured(src);
  az::EmptyReporter reporter;
  az::AstFactory factory;
  az::cfa2::CLICompleter completer;
  az::CompleteLexer lexer(src, len);
  Parser parser(&ctx, &factory, src, &lexer, &reporter, &completer, structured);
  az::FunctionLiteral* const global = parser.ParseProgram();
  assert(global);
  if (completer.HasCompletionPoint()) {
    az::cfa2::Heap heap(&factory, &completer);
    az::cfa2::Complete(global, &heap, src, &reporter);
    completer.Output();
  }
  return EXIT_SUCCESS;
}

inline int Tag(const iv::core::UString& src) {
  typedef az::Parser<iv::core::UString,
                     az::CompleteLexer,
                     az::EmptyReporter,
                     az::BasicCompleter> Parser;
  az::Context ctx;
  az::StructuredSource structured(src);
  az::EmptyReporter reporter;
  az::AstFactory factory;
  az::CompleteLexer lexer(src);
  Parser parser(&ctx, &factory, src, &lexer, &reporter, NULL, structured);
  az::FunctionLiteral* const global = parser.ParseProgram();
  assert(global);
  az::cfa2::Heap heap(&factory, NULL);
  az::cfa2::Complete(global, &heap, src, &reporter);
  return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char** argv) {
  az::DebugLog("PROGRAM START");
  iv::cmdline::Parser cmd("az");
  cmd.Add("help",
          "help",
          'h', "print this message");
  cmd.Add("version",
          "version",
          'v', "print the version");
  cmd.Add<std::size_t>(
      "pulse",
      "pulse",
      0, "pulse option", false, 0);
  cmd.Add(
      "tag",
      "tag",
      't', "tag option");
  cmd.AddList<std::string>(
      "file",
      "file",
      'f', "script file to load");
  cmd.set_footer("[program_file] [arguments]");

  const bool cmd_parse_success = cmd.Parse(argc, argv);
  if (!cmd_parse_success) {
    std::fprintf(stderr, "%s\n%s",
                 cmd.error().c_str(), cmd.usage().c_str());
    return EXIT_FAILURE;
  }

  if (cmd.Exist("help")) {
    std::fputs(cmd.usage().c_str(), stdout);
    return EXIT_SUCCESS;
  }

  const std::vector<std::string>& rest = cmd.rest();
  if (rest.empty() && !cmd.Exist("file")) {
    std::fputs(cmd.usage().c_str(), stdout);
    return EXIT_FAILURE;
  }

  std::vector<char> res;
  if (cmd.Exist("file")) {
    const std::vector<std::string>& vec = cmd.GetList<std::string>("file");
    for (std::vector<std::string>::const_iterator it = vec.begin(),
         last = vec.end(); it != last; ++it) {
      if (!ReadFile(*it, &res)) {
        return EXIT_FAILURE;
      }
    }
  } else {
    if (!ReadFile(rest.front(), &res)) {
      return EXIT_FAILURE;
    }
  }
  iv::core::UString src;
  src.reserve(res.size());
  if (iv::core::unicode::UTF8ToUTF16(
          res.begin(),
          res.end(),
          std::back_inserter(src)) != iv::core::unicode::NO_ERROR) {
    std::fprintf(stderr, "%s\n", "invalid UTF-8 encoding file");
    return EXIT_FAILURE;
  }
  if (cmd.Exist("pulse")) {
    // pulse mode
    const std::size_t len = cmd.Get<std::size_t>("pulse");
    if (len > src.size()) {
      std::fprintf(stderr,
                   "%s %lld %s %lld%s\n",
                   "pulse position",
                   static_cast<long long>(len),
                   "is out of range (size",
                   static_cast<long long>(src.size()),
                   ")");
      return EXIT_FAILURE;
    }
    return Pulse(src, len);
  } else if (cmd.Exist("tag")) {
    return Tag(src);
  } else {
    typedef az::Parser<iv::core::UString,
                       az::CompleteLexer,
                       az::Reporter,
                       az::BasicCompleter> Parser;
    // normal analysis
    az::Context ctx;
    az::StructuredSource structured(src);
    az::Reporter reporter(structured);
    az::AstFactory factory;
    az::CompleteLexer lexer(src);
    Parser parser(&ctx, &factory, src, &lexer, &reporter, NULL, structured);
    az::FunctionLiteral* const global = parser.ParseProgram();
    assert(global);
    az::Analyze(global, src, &reporter);
  }
  return 0;
}
