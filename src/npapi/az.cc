// NPAPI Core
#include <cstdio>
#include <memory>
#include <algorithm>
#include <cstring>
#include <npapi/npapi.h>
#include <npapi/nptypes.h>
#include <npapi/npruntime.h>
#include <npapi/npfunctions.h>
#include <iv/platform.h>
#include <iv/debug.h>
#include "az.h"
namespace {

static az::NPAPI* npapi;
static NPNetscapeFuncs* npnfuncs;

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
#endif

}  // namespace anonymous
namespace az {

bool NPAPI::StringToNPVariant(const std::string &str, NPVariant *variant) {
  const std::size_t len = str.size();
  NPUTF8* chars = static_cast<NPUTF8*>(npnfuncs->memalloc(len));
  if(!chars){
    VOID_TO_NPVARIANT(*variant);
    return false;
  }
  std::copy(str.begin(), str.end(), chars);
  std::memcpy(chars, str.c_str(), len);
  STRINGN_TO_NPVARIANT(chars, len, *variant);
  return true;
}

NPAPI::NPAPI(NPP* instance)
  : instance_(instance) {
  npobject_ = npnfuncs->createobject(*instance_, &NPAPI::kNpcRefObject);
  npnfuncs->retainobject(npobject_);
}

NPAPI::~NPAPI() {
  npnfuncs->releaseobject(npobject_);
}

bool NPAPI::HasMethod(NPObject *obj, NPIdentifier methodName) {
  char* name = npnfuncs->utf8fromidentifier(methodName);
  bool result =
      std::strcmp(name, NPAPI::kMethodPost) == 0 || std::strcmp(name, NPAPI::kMethodCurrentTrack) == 0;
  npnfuncs->memfree(name);
  return result;
}

bool NPAPI::Invoke(NPObject *obj, NPIdentifier methodName,
                   const NPVariant *args, uint32_t argCount, NPVariant *result){
  NPUTF8* name = npnfuncs->utf8fromidentifier(methodName);
  BOOLEAN_TO_NPVARIANT(false, *result);
  if(std::strcmp(name, NPAPI::kMethodPost) == 0){
    npnfuncs->memfree(name);
    if(argCount < 1){
      npnfuncs->setexception(obj, "Parameter 1 is required.");
      return false;
    }
    BOOLEAN_TO_NPVARIANT(true, *result);
    return true;
  } else if(std::strcmp(name, NPAPI::kMethodCurrentTrack) == 0){
    npnfuncs->memfree(name);
    if(argCount > 0){
      npnfuncs->setexception(obj, "Parameter isn't required.");
      return false;
    }
    NULL_TO_NPVARIANT(*result);
    return true;
  } else {
    npnfuncs->memfree(name);
    npnfuncs->setexception(obj, "no such method.");
    return false;
  }
}

}  // namespace az
#ifdef __cplusplus
extern "C" {
#endif

NPError nevv(NPMIMEType pluginType, NPP instance,
             uint16_t mode, int16_t argc, char *argn[], char *argv[], NPSavedData *saved) {
  return NPERR_NO_ERROR;
}

NPError destroy(NPP instance, NPSavedData **save) {
  delete npapi;
  npapi = NULL;
  return NPERR_NO_ERROR;
}

// getValue function when getter called
NPError getValue(NPP instance, NPPVariable variable, void* value) {
  switch(variable){
  case NPPVpluginNameString:
    *(static_cast<const char **>(value)) = "iTunes daemon";
    break;
  case NPPVpluginDescriptionString:
    *(static_cast<const char **>(value)) = "NPAPI extension to post music to iTunes";
    break;
  case NPPVpluginScriptableNPObject:
    if(!npapi){
      npapi = new az::NPAPI(&instance);
    }
    *(NPObject **)value = npapi->NPObjectValue();
    break;
#ifdef XUL_RUNNER_SDK
  case NPPVpluginNeedsXEmbed:
    break;
#endif
  default:
    return NPERR_GENERIC_ERROR;
  }
  return NPERR_NO_ERROR;
}

NPError OSCALL NP_GetEntryPoints(NPPluginFuncs *nppfuncs) {
  nppfuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  nppfuncs->size    = sizeof(*nppfuncs);
  nppfuncs->newp    = nevv;
  nppfuncs->destroy = destroy;
  nppfuncs->getvalue= getValue;
  return NPERR_NO_ERROR;
}

NPError OSCALL NP_Initialize(
    NPNetscapeFuncs *npnf
#if !defined(WIN32) && !defined(WEBKIT_DARWIN_SDK)
    ,NPPluginFuncs *nppfuncs
#endif
) {
  if(npnf == NULL) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }

  if((npnf->version >> 8) > NP_VERSION_MAJOR) {
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  npnfuncs = npnf;
#if !defined(WIN32) && !defined(WEBKIT_DARWIN_SDK)
  NP_GetEntryPoints(nppfuncs);
#endif
  return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown() {
  return NPERR_NO_ERROR;
}

char* NP_GetMIMEDescription() {
  return az::NPAPI::kMIMETypeDescription;
}

NPError OSCALL NP_GetValue(void *npp, NPPVariable variable, void *value) {
  return getValue((NPP)npp, variable, value);
}

#ifdef __cplusplus
}
#endif
