#ifndef _AZ_CFA2_BUILTINS_H_
#define _AZ_CFA2_BUILTINS_H_
#include <az/cfa2/aobject.h>
#include <az/cfa2/aval.h>
#include <az/cfa2/answer.h>
namespace az {
namespace cfa2 {

inline Answer OBJECT_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled) {
  return std::make_tuple(AVal(), false, AVal(AVAL_NOBASE));
}

inline Answer ARRAY_CONSTRUCTOR(Heap* heap,
                                const AVal& this_binding,
                                const std::vector<AVal>& args,
                                bool IsConstructorCalled) {
  // const std::size_t len = args.size();
  if (IsConstructorCalled) {
    // called by new Array() form
    // TODO(Constellation) implement it
    return std::make_tuple(AVal(), false, AVal(AVAL_NOBASE));
  } else {
    // called by Array() form
    // TODO(Constellation) implement it
    AVal val = heap->GetArrayFunctionCalledValue();
    // mutate and return this
    return std::make_tuple(val, false, AVal(AVAL_NOBASE));
  }
}

inline Answer REGEXP_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled) {
  return std::make_tuple(AVal(), false, AVal(AVAL_NOBASE));
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BUILTINS_H_
