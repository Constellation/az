#ifndef _AZ_NPAPI_DEBUG_H_
#define _AZ_NPAPI_DEBUG_H_

#ifdef DEBUG
inline void Log(const char *msg) {
#if defined(OS_WIN)
  std::FILE *out;
  fopen_s(&out, "c:\\az.log", "abN");
#else
  std::FILE *out = std::fopen("/tmp/az.log", "ab");
#endif
  std::fprintf(out, "%s\n", msg);
  std::fclose(out);
}

#else

#define Log(msg)

#endif

#endif  // _AZ_NPAPI_DEBUG_H_
