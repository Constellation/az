#ifndef AZ_CFA2_AVAL_H_
#define AZ_CFA2_AVAL_H_
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/completer.h>
#include <az/cfa2/interpreter_fwd.h>
#include <az/cfa2/type_interpreter_fwd.h>
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
  AlreadySearched already_searched;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val |= (*it)->GetPropertyImpl(name, &already_searched);
  }
  return val;
}

AVal AVal::GetStringProperty(Heap* heap) const {
  AVal val(AVAL_NOBASE);
  AlreadySearched already_searched;
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
      if (FunctionLiteral* literal = (*it)->AsJSFunction()) {
        // jsdoc @extends / @interface check
        // TODO(Constellation) lookup only prototype and set
        if (std::shared_ptr<jsdoc::Info> info = heap->GetInfo(literal)) {
          if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_IMPLEMENTS)) {
            this_binding->UpdatePrototype(
                heap,
                TypeInterpreter::Interpret(heap, tag->type()));
          }
          if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_EXTENDS)) {
            this_binding->UpdatePrototype(
                heap,
                TypeInterpreter::Interpret(heap, tag->type()));
          }
        }
      }
      this_binding->UpdatePrototype(heap, (*it)->GetProperty(Intern("prototype")));
      res |= interp->EvaluateFunction(*it, base, args, true);
    }
  }
  *result = res;
}

AVal AVal::GetPropertyImpl(Symbol name, AlreadySearched* already_searched) const {
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

AVal AVal::GetStringPropertyImpl(AlreadySearched* already_searched) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    if (already_searched->find(*it) == already_searched->end()) {
      already_searched->insert(*it);
      val |= (*it)->GetStringPropertyImpl(already_searched);
    }
  }
  return val;
}

AVal AVal::GetNumberProperty(Heap* heap) const {
  AVal val(AVAL_NOBASE);
  AlreadySearched already_searched;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val |= (*it)->GetNumberPropertyImpl(&already_searched);
  }
  return val;
}

AVal AVal::GetNumberPropertyImpl(AlreadySearched* already_searched) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    if (already_searched->find(*it) == already_searched->end()) {
      already_searched->insert(*it);
      val |= (*it)->GetNumberPropertyImpl(already_searched);
    }
  }
  return val;
}

iv::core::UString AVal::ToTypeString(Heap* heap) const {
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
  for (std::array<Pair, 7>::const_iterator it = kBase.begin(),
       last = kBase.end(); it != last; ++it) {
    if ((it->type & base_) == it->type) {
      // include this base type
      types.insert(iv::core::ToUString(it->str));
    }
  }

  AlreadySearched already_searched;
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    types.insert((*it)->ToTypeString(heap, &already_searched));
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

// start point
void AVal::Complete(Heap* heap, Completer* completer) const {
  AlreadySearched already_searched;
  Complete(heap, completer, &already_searched);
}

void AVal::Complete(Heap* heap, Completer* completer,
                    AlreadySearched* already_searched) const {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->Complete(heap, completer, already_searched);
  }
}

} }  // namespace az::cfa2
#endif  // AZ_CFA2_AVAL_H_
