#ifndef _AZ_CFA2_AOBJECT_H_
#define _AZ_CFA2_AOBJECT_H_
#include <az/cfa2/aobject_fwd.h>
#include <az/cfa2/heap.h>
namespace az {
namespace cfa2 {

void AObject::UpdateProperty(Heap* heap, Symbol name, const AVal& val) {
  const Properties::iterator it = properties_.find(name);
  if (it != properties_.end()) {
    // found
    if (!(val < it->second.value())) {
      it->second.Merge(val);
      heap->UpdateState();  // to new state (object layout change)
    }
  } else {
    AddProperty(name, AProp(val));
    heap->UpdateState();  // to new state (object layout change)
  }
}


void AObject::UpdatePrototype(Heap* heap, const AVal& val) {
  if (!(val < proto_)) {
    proto_ |= val;
    heap->UpdateState();
  }
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AOBJECT_H_
