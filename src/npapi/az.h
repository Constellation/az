#ifndef _AZ_NPAPI_AZ_H_
#define _AZ_NPAPI_AZ_H_
#include <vector>
#include <string>
#include <algorithm>
#include <iv/stringpiece.h>
namespace az {

class NPAPI;

NPAPI* npapi = NULL;
NPNetscapeFuncs* npnfuncs = NULL;

class NPAPI {
 public:
  static const char * const kMethodPost;
  static const char * const kMethodCurrentTrack;
  static NPMIMEType kMIMETypeDescription;
  static struct NPClass kNpcRefObject;
  static struct NPClass kStringHashObject;

  explicit NPAPI(NPP*);
  ~NPAPI();

  inline NPObject* NPObjectValue() const {
    return npobject_;
  }

  inline NPP Instance() const {
    return *instance_;
  }

  static bool StringToNPVariant(const std::string&, NPVariant*);
  static bool HasMethod(NPObject*, NPIdentifier);
  static bool Invoke(NPObject*, NPIdentifier, const NPVariant*, uint32_t, NPVariant*);
  static NPNetscapeFuncs * Npnfuncs;
 private:
  NPP *instance_;
  NPObject * npobject_;
};

NPMIMEType NPAPI::kMIMETypeDescription = const_cast<NPMIMEType>("application/x-chrome-npapi-az-analyzer:.:az-analyzer@Constellation");

struct NPClass NPAPI::kNpcRefObject = {
  NP_CLASS_STRUCT_VERSION,
  NULL,
  NULL,
  NULL,
  NPAPI::HasMethod,
  NPAPI::Invoke,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

inline bool StringToNPVariant(const iv::core::StringPiece& str, NPVariant *variant) {
  const std::size_t len = str.size();
  NPUTF8* chars = static_cast<NPUTF8*>(npnfuncs->memalloc(len));
  if(!chars){
    VOID_TO_NPVARIANT(*variant);
    return false;
  }
  std::copy(str.begin(), str.end(), chars);
  STRINGN_TO_NPVARIANT(chars, len, *variant);
  return true;
}

}  // namespace az
#endif  // _AZ_NPAPI_AZ_H_
