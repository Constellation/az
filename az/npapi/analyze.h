#ifndef _AZ_NPAPI_ANALYZE_H_
#define _AZ_NPAPI_ANALYZE_H_
#include <iv/stringpiece.h>
#include <npapi/npapi.h>
#include <npapi/nptypes.h>
#include <npapi/npruntime.h>
#include <npapi/npfunctions.h>
namespace az {

bool Analyze(NPNetscapeFuncs* np,
             NPObject* receiver, const iv::core::StringPiece& piece, NPVariant* result);

}  // namespace az
#endif  // _AZ_NPAPI_ANALYZE_H_
