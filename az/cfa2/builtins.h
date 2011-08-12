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
  AVal val = (IsConstructorCalled) ?
      this_binding : heap->GetArrayConstructorResult();
  AVal dummy;
  const bool init_with_args = args.size() > 1;
  for (AVal::ObjectSet::iterator it = val.objects().begin(),
       last = val.objects().end(); it != last; ++it) {
    (*it)->UpdatePrototype(heap, heap->GetArrayPrototype());
    if ((*it)->GetOwnProperty(Intern("length"), &dummy)) {
      (*it)->UpdateProperty(heap, Intern("length"), AVal(AVAL_NUMBER));
    } else {
      (*it)->AddProperty(
          Intern("length"), AProp(AVal(AVAL_NUMBER), A::C | A::W));
    }
    if (init_with_args) {
      for (uint32_t index = 0, len = args.size(); index < len; ++index) {
        (*it)->UpdateProperty(heap, Intern(index), args[index]);
      }
    }
  }
  return Result(val);
}

inline Result REGEXP_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled) {
  for (AVal::ObjectSet::iterator it = this_binding.objects().begin(),
       last = this_binding.objects().end(); it != last; ++it) {
    (*it)->UpdatePrototype(heap, heap->GetRegExpPrototype());
    (*it)->UpdateProperty(heap, Intern("global"), AVal(AVAL_BOOL));
    (*it)->UpdateProperty(heap, Intern("ignoreCase"), AVal(AVAL_BOOL));
    (*it)->UpdateProperty(heap, Intern("lastIndex"), AVal(AVAL_NUMBER));
    (*it)->UpdateProperty(heap, Intern("multiline"), AVal(AVAL_BOOL));
    (*it)->UpdateProperty(heap, Intern("source"), AVal(AVAL_STRING));
  }
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

inline Result TO_THIS(Heap* heap,
                      const AVal& this_binding,
                      const std::vector<AVal>& args,
                      bool IsConstructorCalled) {
  return Result(this_binding);
}

inline Result TO_UNDEFINED(Heap* heap,
                           const AVal& this_binding,
                           const std::vector<AVal>& args,
                           bool IsConstructorCalled) {
  return Result(AVal(AVAL_UNDEFINED));
}

inline Result StringMatch(Heap* heap,
                          const AVal& this_binding,
                          const std::vector<AVal>& args,
                          bool IsConstructorCalled) {
  return Result(heap->GetStringMatchResult());
}

inline Result StringSplit(Heap* heap,
                          const AVal& this_binding,
                          const std::vector<AVal>& args,
                          bool IsConstructorCalled) {
  return Result(heap->GetStringSplitResult());
}

inline Result RegExpExec(Heap* heap,
                         const AVal& this_binding,
                         const std::vector<AVal>& args,
                         bool IsConstructorCalled) {
  return Result(heap->GetRegExpExecResult());
}

inline Result JSONParse(Heap* heap,
                        const AVal& this_binding,
                        const std::vector<AVal>& args,
                        bool IsConstructorCalled) {
  return Result(heap->GetJSONParseResult());
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BUILTINS_H_
