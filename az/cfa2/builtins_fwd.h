#ifndef AZ_CFA2_BUILTINS_FWD_H_
#define AZ_CFA2_BUILTINS_FWD_H_
#include <az/cfa2/fwd.h>
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/result.h>
namespace az {
namespace cfa2 {

typedef Result (*Builtin)(Heap* heap,
                          const AVal& this_binding,
                          const std::vector<AVal>& args,
                          bool IsConstructorCalled);

inline Result OBJECT_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled);

inline Result ARRAY_CONSTRUCTOR(Heap* heap,
                                const AVal& this_binding,
                                const std::vector<AVal>& args,
                                bool IsConstructorCalled);

inline Result REGEXP_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled);

inline Result STRING_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled);

inline Result BOOLEAN_CONSTRUCTOR(Heap* heap,
                                  const AVal& this_binding,
                                  const std::vector<AVal>& args,
                                  bool IsConstructorCalled);

inline Result NUMBER_CONSTRUCTOR(Heap* heap,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled);

inline Result DATE_CONSTRUCTOR(Heap* heap,
                               const AVal& this_binding,
                               const std::vector<AVal>& args,
                               bool IsConstructorCalled);

inline Result TO_NUMBER(Heap* heap,
                        const AVal& this_binding,
                        const std::vector<AVal>& args,
                        bool IsConstructorCalled);

inline Result TO_STRING(Heap* heap,
                        const AVal& this_binding,
                        const std::vector<AVal>& args,
                        bool IsConstructorCalled);

inline Result TO_BOOLEAN(Heap* heap,
                         const AVal& this_binding,
                         const std::vector<AVal>& args,
                         bool IsConstructorCalled);

inline Result TO_THIS(Heap* heap,
                      const AVal& this_binding,
                      const std::vector<AVal>& args,
                      bool IsConstructorCalled);

inline Result TO_UNDEFINED(Heap* heap,
                           const AVal& this_binding,
                           const std::vector<AVal>& args,
                           bool IsConstructorCalled);

template<int BASE_TYPE>
inline Result TO_BASE(Heap* heap,
                      const AVal& this_binding,
                      const std::vector<AVal>& args,
                      bool IsConstructorCalled) {
  return Result(AVal(BASE_TYPE));
}

inline Result StringMatch(Heap* heap,
                          const AVal& this_binding,
                          const std::vector<AVal>& args,
                          bool IsConstructorCalled);

inline Result StringSplit(Heap* heap,
                          const AVal& this_binding,
                          const std::vector<AVal>& args,
                          bool IsConstructorCalled);

inline Result RegExpExec(Heap* heap,
                         const AVal& this_binding,
                         const std::vector<AVal>& args,
                         bool IsConstructorCalled);

inline Result JSONParse(Heap* heap,
                        const AVal& this_binding,
                        const std::vector<AVal>& args,
                        bool IsConstructorCalled);

} }  // namespace az::cfa2
#endif  // AZ_CFA2_BUILTINS_FWD_H_
