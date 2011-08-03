#ifndef _AZ_CFA2_AVAL_H_
#define _AZ_CFA2_AVAL_H_
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/aobject.h>
namespace az {
namespace cfa2 {


void AVal::UpdateProperty(Symbol name, const AVal& val) {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->UpdateProperty(name, val);
  }
}

AVal AVal::GetProperty(Symbol name) {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val.Join((*it)->GetProperty(name));
  }
  return val;
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AVAL_H_
