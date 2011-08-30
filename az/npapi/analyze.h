#ifndef AZ_NPAPI_ANALYZE_H_
#define AZ_NPAPI_ANALYZE_H_
#include <iv/stringpiece.h>
#include <npapi/npapi.h>
#include <npapi/nptypes.h>
#include <npapi/npruntime.h>
#include <npapi/npfunctions.h>
namespace az {
namespace npapi {

bool Analyze(NPNetscapeFuncs* np,
             NPObject* receiver, const iv::core::StringPiece& piece, NPVariant* result);

bool Complete(NPNetscapeFuncs* np,
              NPObject* receiver, const iv::core::StringPiece& piece, std::size_t len, NPVariant* result);

} }  // namespace az::npapi
#endif  // AZ_NPAPI_ANALYZE_H_
