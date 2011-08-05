#ifndef _AZ_CFA2_BUILTINS_H_
#define _AZ_CFA2_BUILTINS_H_
#include <az/cfa2/aobject.h>
#include <az/cfa2/aval.h>
namespace az {
namespace cfa2 {


AVal OBJECT_CONSTRUCTOR(const AVal& this_binding,
                        const std::vector<AVal>& args) {
  return AVal();
}


AVal ARRAY_CONSTRUCTOR(const AVal& this_binding,
                       const std::vector<AVal>& args) {
  return AVal();
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BUILTINS_H_
