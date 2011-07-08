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
#include "parser.h"
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
    std::fprintf(stderr, "%s\n", "invalid UTF-8 encoding file");
    return EXIT_FAILURE;
  }
  az::StructuredSource structured(src);
  az::Reporter reporter(structured);
  az::AstFactory factory;
  Parser parser(&factory, src, &reporter, structured);
  az::FunctionLiteral* const global = parser.ParseProgram();
  if (!global) {
    // syntax error occurred
    std::fprintf(stderr, "%s\n", parser.errors().back().c_str());
  } else {
    az::Analyze(global, src, &reporter);
  }
  return 0;
}
