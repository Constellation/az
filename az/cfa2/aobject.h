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
    if (string_) {
      // absorb to string_
      UpdateStringProperty(heap, val);
    } else if (number_ && IsArrayIndexSymbol(name)) {
      UpdateNumberProperty(heap, val);
    } else {
      AddProperty(name, AProp(val));
      heap->UpdateState();  // to new state (object layout change)
    }
  }
}


void AObject::UpdatePrototype(Heap* heap, const AVal& val) {
  if (!(val < proto_)) {
    proto_ |= val;
    heap->UpdateState();
  }
}

AVal AObject::GetNumberProperty(Heap* heap) {
  MergeNumberProperty(heap);
  return *number_;
}

AVal AObject::GetNumberPropertyImpl(std::unordered_set<const AObject*>* already_searched) const {
  if (number_) {
    return *number_;
  } else {
    return proto_.GetNumberPropertyImpl(already_searched);
  }
}

void AObject::MergeNumberProperty(Heap* heap) {
  if (number_) {
    return;
  }
  number_ = std::shared_ptr<AVal>(new AVal(AVAL_NOBASE));
  heap->UpdateState();
}

void AObject::UpdateNumberProperty(Heap* heap, const AVal& val) {
  MergeNumberProperty(heap);
  if (!(val < *number_)) {
    number_->Join(val);
    heap->UpdateState();  // to new state (object layout change)
  }
}

AVal AObject::GetStringProperty(Heap* heap) {
  if (string_) {
    return *string_;
  } else {
    // TODO(Constellation) implement it
    return AVal(AVAL_NOBASE);
  }
}

AVal AObject::GetStringPropertyImpl(std::unordered_set<const AObject*>* already_searched) const {
  already_searched->insert(this);
  if (string_) {
    return *string_;
  } else {
    return proto_.GetStringPropertyImpl(already_searched);
  }
}

void AObject::MergeStringProperty(Heap* heap) {
  if (string_) {
    return;
  }
  MergeNumberProperty(heap);
  string_ = std::shared_ptr<AVal>(new AVal(AVAL_NOBASE));
  heap->UpdateState();
}

void AObject::UpdateStringProperty(Heap* heap, const AVal& val) {
  MergeNumberProperty(heap);
  MergeStringProperty(heap);
  UpdateNumberProperty(heap, val);
  if (!(val < *string_)) {
    string_->Join(val);
    heap->UpdateState();  // to new state (object layout change)
  }
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AOBJECT_H_
