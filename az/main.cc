#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <iterator>
#include <iv/detail/array.h>
#include <iv/detail/cinttypes.h>
#include <iv/parser.h>
#include <iv/ustring.h>
#include <iv/unicode.h>
#include <iv/cmdline.h>
#include <iv/debug.h>
#include <iv/scoped_ptr.h>
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
#include <fstream>
namespace {

bool ReadFile(const std::string& filename, std::vector<char>* out) {
#if 0
	std::ifstream ifs(filename.c_str(), std::ios::binary);
	if (!ifs) {
	    std::string err = "az can't open \"" + filename + "\"";
		std::perror(err.c_str());
		return false;
	}
	ifs.seekg(0, std::ifstream::end);
	const size_t size = ifs.tellg();
	if (size == 0) {
		return true;
	}
	ifs.seekg(0);
	out->resize(size);
	ifs.read(&(*out)[0], size);
	return true;
#else
  if (std::FILE* fp = std::fopen(filename.c_str(), "rb")) {
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
#endif
}

inline bool ParsePulseOption(const std::string& format,
                             std::pair<std::size_t, std::size_t>* res) {
  unsigned int line;
  unsigned int column;
  if (std::sscanf(  // NOLINT
          format.c_str(),
          "%u:%u", &line, &column) != 2) {
    return false;
  }
  res->first = line;
  res->second = column;
  return true;
}

inline int Pulse(const std::vector<char>& preload,
                 const iv::core::UString& script,
                 const std::string& format,
                 bool for_in_handling) {
  typedef az::Parser<iv::core::UString,
                     az::CompleteLexer,
                     az::EmptyReporter,
                     az::BasicCompleter> Parser;
  std::pair<std::size_t, std::size_t> res;
  if (!ParsePulseOption(format, &res)) {
    std::fprintf(stderr, "%s: %s\n", "invalid pulse option", format.c_str());
    return EXIT_FAILURE;
  }

  std::size_t offset = 0;
  {
    // calculating offset...
    const az::StructuredSource structured_script(script);

    if (!structured_script.InRange(res.first, res.second)) {
      std::fprintf(stderr, "%s\n", "pulse offset is out of range");
      return EXIT_FAILURE;
    }
    offset = structured_script.GetOffset(res.first, res.second);
  }

  // load preload
  iv::core::UString src;
  src.reserve(preload.size() + script.size());
  if (iv::core::unicode::UTF8ToUTF16(
          preload.begin(),
          preload.end(),
          std::back_inserter(src)) != iv::core::unicode::UNICODE_NO_ERROR) {
    std::fprintf(stderr, "%s\n", "invalid UTF-8 encoding file");
    return EXIT_FAILURE;
  }
  src.append(script);
  const az::StructuredSource structured(src);
  az::EmptyReporter reporter;

  iv::core::ScopedPtr<az::AstFactory> factory(new az::AstFactory());
  az::cfa2::CLICompleter completer;
  az::CompleteLexer lexer(src, offset);
  az::cfa2::Heap ctx;
  Parser parser(&ctx, factory.get(), src, &lexer, &reporter, &completer, structured);
  az::FunctionLiteral* const global = parser.ParseProgram();
  assert(global);
  if (completer.HasCompletionPoint()) {
    ctx.InitializeCFA2(factory.get(), &completer, for_in_handling);
    az::cfa2::Complete(global, &ctx, src, &reporter);
    completer.Output();
  }
  return EXIT_SUCCESS;
}

inline int Tag(const iv::core::UString& src, bool for_in_handling) {
  typedef az::Parser<iv::core::UString,
                     az::CompleteLexer,
                     az::EmptyReporter,
                     az::BasicCompleter> Parser;
  az::cfa2::Heap ctx;
  az::StructuredSource structured(src);
  az::EmptyReporter reporter;
  az::AstFactory factory;
  az::CompleteLexer lexer(src);
  Parser parser(&ctx, &factory, src, &lexer, &reporter, NULL, structured);
  az::FunctionLiteral* const global = parser.ParseProgram();
  assert(global);
  ctx.InitializeCFA2(&factory, NULL, for_in_handling);
  az::cfa2::Complete(global, &ctx, src, &reporter);
  return EXIT_SUCCESS;
}

}  // namespace

int main(int argc, char** argv) {
  az::DebugLog("\nPROGRAM START");
  iv::cmdline::Parser cmd("az");
  cmd.Add("help",
          "help",
          'h', "print this message");
  cmd.Add("version",
          "version",
          'v', "print the version");
  cmd.Add<std::string>(
      "pulse",
      "pulse",
      0, "pulse option");
  cmd.Add(
      "for-in-handling",
      "for-in-handling",
      0, "enable for-in-handling in cfa2 (too heavy)");
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
  if (rest.empty()) {
    std::fputs(cmd.usage().c_str(), stdout);
    return EXIT_FAILURE;
  }

  // preload files
  std::vector<char> res;
  if (cmd.Exist("file")) {
    const std::vector<std::string>& vec = cmd.GetList<std::string>("file");
    for (std::vector<std::string>::const_iterator it = vec.begin(),
         last = vec.end(); it != last; ++it) {
      if (!ReadFile(*it, &res)) {
        return EXIT_FAILURE;
      }
    }
  }

  if (cmd.Exist("pulse")) {
    // pulse mode
    const std::string format = cmd.Get<std::string>("pulse");

    std::vector<char> script;
    if (!ReadFile(rest.front(), &script)) {
      return EXIT_FAILURE;
    }

    iv::core::UString script_source;
    script_source.reserve(script.size());
    if (iv::core::unicode::UTF8ToUTF16(
            script.begin(),
            script.end(),
            std::back_inserter(script_source)) != iv::core::unicode::UNICODE_NO_ERROR) {
      std::fprintf(stderr, "%s\n", "invalid UTF-8 encoding file");
      return EXIT_FAILURE;
    }
    int ret = Pulse(res, script_source, format, cmd.Exist("for-in-handling"));
	return ret;
  } else {
    if (!ReadFile(rest.front(), &res)) {
      return EXIT_FAILURE;
    }
    iv::core::UString src;
    src.reserve(res.size());
    if (iv::core::unicode::UTF8ToUTF16(
            res.begin(),
            res.end(),
            std::back_inserter(src)) != iv::core::unicode::UNICODE_NO_ERROR) {
      std::fprintf(stderr, "%s\n", "invalid UTF-8 encoding file");
      return EXIT_FAILURE;
    }
    if (cmd.Exist("tag")) {
      return Tag(src, cmd.Exist("for-in-handling"));
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
  }
  return 0;
}
