#ifndef _AZ_CFA2_BUILTINS_H_
#define _AZ_CFA2_BUILTINS_H_
#include <az/cfa2/aobject.h>
#include <az/cfa2/aval.h>
namespace az {
namespace cfa2 {

inline AVal OBJECT_CONSTRUCTOR(Heap* heap,
                               const AVal& this_binding,
                               const std::vector<AVal>* args,
                               bool IsConstructorCalled) {
  return AVal();
}

inline AVal ARRAY_CONSTRUCTOR(Heap* heap,
                             const AVal& this_binding,
                             const std::vector<AVal>* args,
                             bool IsConstructorCalled) {
  std::size_t len = 0;
  if (args) {
    len = args->size();
  }
  if (IsConstructorCalled) {
    // called by new Array() form
    // TODO(Constellation) implement it
    return AVal();
  } else {
    // called by Array() form
    // TODO(Constellation) implement it
    AVal val = heap->GetArrayFunctionCalledValue();
    // mutate and return this
    return val;
  }
}

inline AVal REGEXP_CONSTRUCTOR(Heap* heap,
                               const AVal& this_binding,
                               const std::vector<AVal>* args,
                               bool IsConstructorCalled) {
  return AVal();
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BUILTINS_H_
