// NPAPI Core
#include <cstdio>
#include <memory>
#include <cstring>
#include <npapi/npapi.h>
#include <npapi/nptypes.h>
#include <npapi/npruntime.h>
#include <npapi/npfunctions.h>
#include <iv/platform.h>
#include <iv/debug.h>
#include "az.h"
namespace {

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

}  // namespace anonymous
namespace az {

NPAPI::NPAPI(NPP* instance)
  : instance_(instance) {
  npobject_ = npnfuncs->createobject(*instance_, &NPAPI::kNpcRefObject);
  npnfuncs->retainobject(npobject_);
}

NPAPI::~NPAPI() {
  npnfuncs->releaseobject(npobject_);
}

bool NPAPI::HasMethod(NPObject *obj, NPIdentifier methodName) {
  Log("HasMethod");
  NPUTF8* name = npnfuncs->utf8fromidentifier(methodName);
  const std::string target(name);
  npnfuncs->memfree(name);
  if (target == "analyze") {
    return true;
  }
  return false;
}

bool NPAPI::Invoke(NPObject *obj, NPIdentifier methodName,
                   const NPVariant *args, uint32_t argCount, NPVariant *result){
  Log("Invoke");
  NPUTF8* name = npnfuncs->utf8fromidentifier(methodName);
  const std::string target(name);
  npnfuncs->memfree(name);
  if (target == "analyze") {
    BOOLEAN_TO_NPVARIANT(false, *result);
    return true;
  } else {
    npnfuncs->setexception(obj, "no such method.");
  }
  return false;
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
  delete az::npapi;
  az::npapi = NULL;
  return NPERR_NO_ERROR;
}

// getValue function when getter called
NPError getValue(NPP instance, NPPVariable variable, void* value) {
  switch(variable){
    case NPPVpluginNameString:
      *(static_cast<const char**>(value)) = "AzAnalyzer";
      break;
    case NPPVpluginDescriptionString:
      *(static_cast<const char**>(value)) = "description";
      break;
    case NPPVpluginScriptableNPObject:
      if(!az::npapi){
        az::npapi = new az::NPAPI(&instance);
      }
      *static_cast<NPObject **>(value) = az::npapi->NPObjectValue();
      break;
    case NPPVpluginNeedsXEmbed:
      *static_cast<bool*>(value) = true;
      break;
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
  Log("Initialize");
  if(npnf == NULL) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }

  if((npnf->version >> 8) > NP_VERSION_MAJOR) {
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  az::npnfuncs = npnf;
#if !defined(WIN32) && !defined(WEBKIT_DARWIN_SDK)
  NP_GetEntryPoints(nppfuncs);
#endif
  return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown() {
  return NPERR_NO_ERROR;
}

char* OSCALL NP_GetMIMEDescription() {
  return az::NPAPI::kMIMETypeDescription;
}

NPError OSCALL NP_GetValue(void *npp, NPPVariable variable, void *value) {
  return getValue((NPP)npp, variable, value);
}

#ifdef __cplusplus
}
#endif
