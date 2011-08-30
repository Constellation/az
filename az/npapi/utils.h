#ifndef AZ_NPAPI_UTILS_H_
#define AZ_NPAPI_UTILS_H_
#include <string>
#include <vector>
#include <algorithm>
#include <iv/stringpiece.h>
#include <iv/ustringpiece.h>
#include <iv/unicode.h>
namespace az {

inline bool StringToNPVariant(NPNetscapeFuncs* np, const iv::core::StringPiece& str, NPVariant* variant) {
  const std::size_t len = str.size();
  NPUTF8* chars = static_cast<NPUTF8*>(np->memalloc(len));
  if(!chars){
    VOID_TO_NPVARIANT(*variant);
    return false;
  }
  std::copy(str.begin(), str.end(), chars);
  STRINGN_TO_NPVARIANT(chars, len, *variant);
  return true;
}

inline bool UStringToNPVariant(NPNetscapeFuncs* np, const iv::core::UStringPiece& str, NPVariant* variant) {
  std::vector<char> vec;
  vec.reserve(str.size());
  if (iv::core::unicode::UTF16ToUTF8(str.begin(),
                                     str.end(),
                                     std::back_inserter(vec)) != iv::core::unicode::NO_ERROR) {
    // invalid UTF16 sequence
    vec.clear();
  }
  return StringToNPVariant(np, iv::core::StringPiece(vec.data(), vec.size()), variant);
}

}  // namespace az
#endif  // AZ_NPAPI_UTILS_H_
