#ifndef _AZ_DEBUG_LOG_H_
#define _AZ_DEBUG_LOG_H_
#include <cstdio>
#include <iv/debug.h>
#include <iv/unicode.h>
#include <iv/stringpiece.h>
#include <iv/ustringpiece.h>
namespace az {

#ifdef DEBUG
static const char* kDebugLogFileName = "/tmp/az.log";

inline void DebugLog(const iv::core::StringPiece& str) {
  if (std::FILE* fp = std::fopen(kDebugLogFileName, "a")) {
    const std::size_t rv = std::fwrite(str.data(), 1, str.size(), fp);
    if (rv == str.size()) {
      std::fputc('\n', fp);
    }
    std::fclose(fp);
  }
}

inline void DebugLog(const iv::core::UStringPiece& us) {
  std::string str;
  str.reserve(us.size());
  iv::core::unicode::UTF16ToUTF8(us, std::back_inserter(str));
  DebugLog(str);
}
#else
template<typename T>
inline void DebugLog(const T& t) { }
#endif

}  // namespace az
#endif  // _AZ_DEBUG_LOG_H_
