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
  // TODO(Constellation) more fast code
  this_binding.UpdatePrototype(heap, heap->GetRegExpPrototype());
  this_binding.UpdateProperty(heap, Intern("global"), AVal(AVAL_BOOL));
  this_binding.UpdateProperty(heap, Intern("ignoreCase"), AVal(AVAL_BOOL));
  this_binding.UpdateProperty(heap, Intern("lastIndex"), AVal(AVAL_NUMBER));
  this_binding.UpdateProperty(heap, Intern("multiline"), AVal(AVAL_BOOL));
  this_binding.UpdateProperty(heap, Intern("source"), AVal(AVAL_STRING));
  return Result(this_binding);
}

inline Result STRING_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled) {
  if (IsConstructorCalled) {
    this_binding.UpdatePrototype(heap, heap->GetStringPrototype());
    return Result(this_binding);
  } else {
    // return string
    return Result(AVal(AVAL_STRING));
  }
}

inline Result BOOLEAN_CONSTRUCTOR(Heap* heap,
                                  const AVal& this_binding,
                                  const std::vector<AVal>& args,
                                  bool IsConstructorCalled) {
  if (IsConstructorCalled) {
    this_binding.UpdatePrototype(heap, heap->GetBooleanPrototype());
    return Result(this_binding);
  } else {
    // return number
    if (this_binding.IsTrue()) {
      return Result(AVal(AVAL_TRUE));
    } else if (this_binding.IsFalse()) {
      return Result(AVal(AVAL_FALSE));
    } else {
      return Result(AVal(AVAL_BOOL));
    }
  }
}

inline Result NUMBER_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled) {
  if (IsConstructorCalled) {
    this_binding.UpdatePrototype(heap, heap->GetNumberPrototype());
    return Result(this_binding);
  } else {
    // return number
    return Result(AVal(AVAL_NUMBER));
  }
}

inline Result DATE_CONSTRUCTOR(Heap* heap,
                               const AVal& this_binding,
                               const std::vector<AVal>& args,
                               bool IsConstructorCalled) {
  if (IsConstructorCalled) {
    this_binding.UpdatePrototype(heap, heap->GetDatePrototype());
    return Result(this_binding);
  } else {
    // return string
    return Result(AVal(AVAL_STRING));
  }
}

inline Result TO_NUMBER(Heap* heap,
                        const AVal& this_binding,
                        const std::vector<AVal>& args,
                        bool IsConstructorCalled) {
  return Result(AVal(AVAL_NUMBER));
}

inline Result TO_STRING(Heap* heap,
                        const AVal& this_binding,
                        const std::vector<AVal>& args,
                        bool IsConstructorCalled) {
  return Result(AVal(AVAL_STRING));
}

inline Result TO_BOOLEAN(Heap* heap,
                         const AVal& this_binding,
                         const std::vector<AVal>& args,
                         bool IsConstructorCalled) {
  return Result(AVal(AVAL_BOOL));
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BUILTINS_H_
