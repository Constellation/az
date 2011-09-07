#ifndef AZ_CFA2_AOBJECT_H_
#define AZ_CFA2_AOBJECT_H_
#include <az/cfa2/type_utils.h>
#include <az/cfa2/aobject_fwd.h>
#include <az/cfa2/heap.h>
#include <az/cfa2/already_searched.h>
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

AVal AObject::GetNumberPropertyImpl(AlreadySearched* already_searched) const {
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
  AVal result(AVAL_NOBASE);
  Properties prop;
  for (Properties::const_iterator it = properties_.begin(),
       last = properties_.end(); it != last; ++it) {
    if (IsArrayIndexSymbol(it->first)) {
      result |= it->second.value();
    } else {
      prop.insert(*it);
    }
  }
  properties_ = prop;
  number_ = std::shared_ptr<AVal>(new AVal(result));
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
    AVal result(AVAL_NOBASE);
    for (Properties::const_iterator it = properties_.begin(),
         last = properties_.end(); it != last; ++it) {
      if (it->second.IsEnumerable()) {
        result |= it->second.value();
      }
    }
    return result;
  }
}

AVal AObject::GetStringPropertyImpl(AlreadySearched* already_searched) const {
  already_searched->insert(this);
  if (string_) {
    return *string_;
  } else {
    AVal result(AVAL_NOBASE);
    for (Properties::const_iterator it = properties_.begin(),
         last = properties_.end(); it != last; ++it) {
      if (it->second.IsEnumerable()) {
        result |= it->second.value();
      }
    }
    const AVal res = proto_.GetStringPropertyImpl(already_searched);
    return result | res;
  }
}

void AObject::MergeStringProperty(Heap* heap) {
  if (string_) {
    return;
  }
  MergeNumberProperty(heap);
  AVal result(AVAL_NOBASE);
  Properties prop;
  for (Properties::const_iterator it = properties_.begin(),
       last = properties_.end(); it != last; ++it) {
    if (it->second.IsEnumerable()) {
      result |= it->second.value();
    } else {
      prop.insert(*it);
    }
  }
  properties_ = prop;
  string_ = std::shared_ptr<AVal>(new AVal(result));
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


void AObject::Complete(Heap* heap, Completer* completer,
                       AlreadySearched* already_searched) const {
  if (already_searched->find(this) == already_searched->end()) {
    already_searched->insert(this);
    for (Properties::const_iterator it = properties_.begin(),
         last = properties_.end(); it != last; ++it) {
      completer->Notify(it->first, it->second.value());
    }
    proto_.ToObject(heap).Complete(heap, completer, already_searched);
  }
}

iv::core::UString AObject::ToTypeString(Heap* heap,
                                        AlreadySearched* already_searched) const {
  if (already_searched->find(this) != already_searched->end()) {
    return iv::core::ToUString("any");
  } else {
    already_searched->insert(this);
  }
  if (IsFunction()) {
    if (builtin_) {
      // this is builtin function
      return iv::core::ToUString("builtin");
    }
    return GetFunctionPrototypeDeclaration(function_);
  }
  const AVal constructor = GetProperty(Intern("constructor"));
  if (constructor.IsUndefined()) {
    // constructor not found
    return iv::core::ToUString("Global");
  }
  return iv::core::ToUString("Object");
}

} }  // namespace az::cfa2
#endif  // AZ_CFA2_AOBJECT_H_
