#ifndef _AZ_CFA2_AVAL_H_
#define _AZ_CFA2_AVAL_H_
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/interpreter_fwd.h>
#include <az/cfa2/aobject_fwd.h>
#include <az/cfa2/result.h>
namespace az {
namespace cfa2 {


void AVal::UpdateProperty(Heap* heap, Symbol name, const AVal& val) const {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->UpdateProperty(heap, name, val);
  }
}

void AVal::UpdateStringProperty(Heap* heap, const AVal& val) const {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->UpdateStringProperty(heap, val);
  }
}

void AVal::UpdateNumberProperty(Heap* heap, const AVal& val) const {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->UpdateNumberProperty(heap, val);
  }
}

void AVal::UpdatePrototype(Heap* heap, const AVal& val) const {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->UpdatePrototype(heap, val);
  }
}

AVal AVal::GetProperty(Heap* heap, Symbol name) const {
  AVal val(AVAL_NOBASE);
  std::unordered_set<const AObject*> already_searched;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val |= (*it)->GetPropertyImpl(name, &already_searched);
  }
  return val;
}

AVal AVal::GetStringProperty(Heap* heap) const {
  AVal val(AVAL_NOBASE);
  std::unordered_set<const AObject*> already_searched;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val |= (*it)->GetStringPropertyImpl(&already_searched);
  }
  return val;
}

AVal AVal::ToObject(Heap* heap) const {
  // convert STRING / NUMBER / BOOL => String / Number / Boolean object
  if (base_ == AVAL_NOBASE) {
    return *this;
  }
  AVal result(AVAL_NOBASE);
  result.objects_ = objects_;
  if (base_ & AVAL_STRING) {
    result |= heap->GetStringObject();
  }
  if (base_ & (AVAL_TRUE | AVAL_FALSE | AVAL_BOOL)) {
    result |= heap->GetBooleanObject();
  }
  if (base_ & AVAL_NUMBER) {
    result |= heap->GetNumberObject();
  }
  return result;
}

void AVal::Call(Heap* heap,
                Interpreter* interp,
                const AVal& this_binding,
                const std::vector<AVal>& args, Result* result) const {
  Result res;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    if ((*it)->IsFunction()) {
      // merge result values
      res |= interp->EvaluateFunction(*it, this_binding, args, false);
    }
  }
  *result = res;
}

void AVal::Construct(Heap* heap,
                     Interpreter* interp,
                     AObject* this_binding,
                     const std::vector<AVal>& args, Result* result) const {
  const AVal base(this_binding);
  Result res;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    if ((*it)->IsFunction()) {
      this_binding->UpdatePrototype(heap, (*it)->GetProperty(Intern("prototype")));
      res |= interp->EvaluateFunction(*it, base, args, true);
    }
  }
  *result = res;
}

AVal AVal::GetPropertyImpl(Symbol name, std::unordered_set<const AObject*>* already_searched) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    if (already_searched->find(*it) == already_searched->end()) {
      val |= (*it)->GetPropertyImpl(name, already_searched);
    }
  }
  return val;
}

AVal AVal::GetStringPropertyImpl(std::unordered_set<const AObject*>* already_searched) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    if (already_searched->find(*it) == already_searched->end()) {
      val |= (*it)->GetStringPropertyImpl(already_searched);
    }
  }
  return val;
}

AVal AVal::GetNumberProperty(Heap* heap) const {
  AVal val(AVAL_NOBASE);
  std::unordered_set<const AObject*> already_searched;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val |= (*it)->GetNumberPropertyImpl(&already_searched);
  }
  return val;
}

AVal AVal::GetNumberPropertyImpl(std::unordered_set<const AObject*>* already_searched) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    if (already_searched->find(*it) == already_searched->end()) {
      val |= (*it)->GetNumberPropertyImpl(already_searched);
    }
  }
  return val;
}

iv::core::UString AVal::ToTypeString() const {
  static const std::array<Pair, 7> kBase = { {
    { 1, "number" },
    { 2, "string" },
    { 4, "bool" },   // true
    { 8, "bool" },   // false
    { 16, "bool" },  // bool
    { 32, "undefined" },
    { 64, "null" }
  } };
  std::set<iv::core::UString> types;
  iv::core::UString str;
  // base type searching
  for (std::array<Pair, 5>::const_iterator it = kBase.begin(),
       last = kBase.end(); it != last; ++it) {
    if ((it->type & base_) == it->type) {
      // include this base type
      types.insert(iv::core::ToUString(it->str));
    }
  }

  std::unordered_set<const AObject*> already_searched;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    types.insert((*it)->ToTypeString(&already_searched));
  }

  if (types.empty()) {
    return iv::core::ToUString("any");
  }
  if (types.size() == 1) {
    return *types.begin();
  }
  str.push_back('<');
  for (std::set<iv::core::UString>::const_iterator it = types.begin(),
       last = types.end();;) {
    str.append(*it);
    ++it;
    if (it != last) {
      str.push_back('|');
    } else {
      break;
    }
  }
  str.push_back('>');
  return str;
}


bool AVal::IsTrue() const {
  return
      objects_.empty() && ((base_ == AVAL_STRING && (str_ && !str_->empty())) || base_ == AVAL_TRUE);
}

bool AVal::IsFalse() const {
  return
      objects_.empty() && base_ != 0 && (((base_ == AVAL_NULL || base_ == AVAL_UNDEFINED || base_ == AVAL_FALSE)) || (base_ == AVAL_STRING && str_ && str_->empty()));
}

void AVal::Complete(Heap* heap, Completer* completer) const {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->Complete(heap, completer);
  }
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AVAL_H_
