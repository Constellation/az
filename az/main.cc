#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iterator>
#include <iv/parser.h>
#include <iv/detail/array.h>
#include <iv/ustring.h>
#include <iv/unicode.h>
#include <iv/cmdline.h>
#include <iv/debug.h>
#include <az/factory.h>
#include <az/analyzer.h>
#include <az/reporter.h>
#include <az/parser.h>
#include <az/symbol.h>
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

}  // namespace

int main(int argc, char** argv) {
  typedef az::Parser<iv::core::UString, az::Reporter> Parser;

  iv::cmdline::Parser cmd("az");
  cmd.Add("help",
          "help",
          'h', "print this message");
  cmd.Add("version",
          "version",
          'v', "print the version");
  cmd.Add("pulse",
          "pulse",
          0, "pulse option");
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
  if (rest.empty()) {
    std::fputs(cmd.usage().c_str(), stdout);
    return EXIT_FAILURE;
  }

  std::vector<char> vec;
  if (!ReadFile(rest.front(), &vec)) {
    return EXIT_FAILURE;
  }

  iv::core::UString src;
  src.reserve(vec.size());
  if (iv::core::unicode::UTF8ToUTF16(
          vec.begin(),
          vec.end(),
          std::back_inserter(src)) != iv::core::unicode::NO_ERROR) {
    std::fprintf(stderr, "%s\n", "invalid UTF-8 encoding file");
    return EXIT_FAILURE;
  }
  az::StructuredSource structured(src);
  az::Reporter reporter(structured);
  az::AstFactory factory;
  Parser parser(&factory, src, &reporter, structured);
  az::FunctionLiteral* const global = parser.ParseProgram();
  assert(global);
  if (cmd.Exist("pulse")) {
    // pulse mode
    az::cfa2::Completer completer;
    az::cfa2::Complete(global, src, &factory, &reporter, &completer);
  } else {
    // normal analysis
    az::Analyze(global, src, &reporter);
  }
  return 0;
}
