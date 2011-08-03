// CFA2 analyze implementation
// original report is
//   http://www.ccs.neu.edu/home/dimvar/papers/cfa2-NU-CCIS-10-01.pdf
// and highly inspired from doctorjs src code
#ifndef _AZ_CFA2_H_
#define _AZ_CFA2_H_
#include <az/cfa2/heap.h>
#include <az/cfa2/binding_resolver.h>
namespace az {
namespace cfa2 {

template<typename Source, typename Reporter>
inline void Complete(FunctionLiteral* global, const Source& src, Reporter* reporter) {
  Heap heap;
  BindingResolver resolver(&heap);
  resolver.Resolve(global);
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_H_
