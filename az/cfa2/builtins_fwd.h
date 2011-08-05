#ifndef _AZ_CFA2_BUILTINS_FWD_H_
#define _AZ_CFA2_BUILTINS_FWD_H_
#include <az/cfa2/aobject.h>
#include <az/cfa2/aval.h>
#include <az/cfa2/answer.h>
namespace az {
namespace cfa2 {

inline Answer OBJECT_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled);

inline Answer ARRAY_CONSTRUCTOR(Heap* heap,
                                const AVal& this_binding,
                                const std::vector<AVal>& args,
                                bool IsConstructorCalled);

inline Answer REGEXP_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled);

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BUILTINS_FWD_H_
