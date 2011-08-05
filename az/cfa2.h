// CFA2 based JS completion implementation
//
// CFA2 original report is
//   http://www.ccs.neu.edu/home/dimvar/papers/cfa2-NU-CCIS-10-01.pdf
// and highly inspired from doctorjs src code
//   http://doctorjs.org/
//
#ifndef _AZ_CFA2_H_
#define _AZ_CFA2_H_
#include <az/cfa2/heap.h>
#include <az/cfa2/aval.h>
#include <az/cfa2/binding_resolver.h>
#include <az/cfa2/interpreter.h>
#include <az/cfa2/completer.h>
namespace az {
namespace cfa2 {

template<typename Source, typename Reporter, typename Completer>
inline void Complete(FunctionLiteral* global,
                     const Source& src, Reporter* reporter, Completer* completer) {
  // initialize heap
  Heap heap;
  {
    // resolve binding type
    BindingResolver resolver(&heap);
    resolver.Resolve(global);
  }
  {
    // execute abstract interpreter
    Interpreter interp(&heap, completer);
    interp.Run(global);
  }
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_H_
