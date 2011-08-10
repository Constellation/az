#ifndef _AZ_CFA2_BUILTINS_H_
#define _AZ_CFA2_BUILTINS_H_
#include <az/cfa2/aobject_fwd.h>
#include <az/cfa2/aval.h>
#include <az/cfa2/result.h>
namespace az {
namespace cfa2 {

inline Result OBJECT_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled) {
  return Result(AVal());
}

inline Result ARRAY_CONSTRUCTOR(Heap* heap,
                                const AVal& this_binding,
                                const std::vector<AVal>& args,
                                bool IsConstructorCalled) {
  // const std::size_t len = args.size();
  if (IsConstructorCalled) {
    // called by new Array() form
    // TODO(Constellation) implement it
    return Result(AVal());
  } else {
    // called by Array() form
    // TODO(Constellation) implement it
    AVal val = heap->GetArrayFunctionCalledValue();
    // mutate and return this
    return Result(val);
  }
}

inline Result REGEXP_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled) {
  return Result(AVal());
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BUILTINS_H_
