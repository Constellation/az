#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <iterator>
#include <iv/parser.h>
#include <iv/detail/array.h>
#include <iv/ustring.h>
#include <iv/unicode.h>
#include "factory.h"
#include "analyzer.h"
#include "reporter.h"
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
  typedef iv::core::Parser<az::AstFactory, iv::core::UString, true, false> Parser;
  if (argc <= 1) {
    std::fprintf(stderr, "%s\n", "filename requred");
    return EXIT_FAILURE;
  }

  std::vector<char> vec;
  if (!ReadFile(argv[1], &vec)) {
    return EXIT_FAILURE;
  }

  iv::core::UString src;
  src.reserve(vec.size());
  if (iv::core::unicode::UTF8ToUTF16(
          vec.begin(),
          vec.end(),
          std::back_inserter(src)) != iv::core::unicode::NO_ERROR) {
    return EXIT_FAILURE;
  }
  az::AstFactory factory;
  Parser parser(&factory, src);
  az::FunctionLiteral* const global = parser.ParseProgram();
  if (!global) {
    // syntax error occurred
    std::fprintf(stderr, "%s\n", parser.error().c_str());
  } else {
    az::Reporter reporter(src);
    az::Analyze(global, src, &reporter);
  }
  return 0;
}
