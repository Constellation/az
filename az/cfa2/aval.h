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

AVal AVal::GetProperty(Heap* heap, Symbol name) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val |= (*it)->GetProperty(name);
  }
  return val;
}

void AVal::Call(Heap* heap,
                Interpreter* interp,
                const AVal& this_binding,
                const std::vector<AVal>& args, Result* result) const {
  AVal res(AVAL_NOBASE);
  bool error_found = false;
  AVal err(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    if ((*it)->IsFunction()) {
      const Result ret = interp->EvaluateFunction(*it, this_binding, args, false);
      // merge result values
      res |= ret.result();
      if (ret.HasException()) {
        error_found = true;
        err |= ret.exception();
      }
    }
  }
  *result = Result(res, err, error_found);
}

void AVal::Construct(Heap* heap,
                     Interpreter* interp,
                     AObject* this_binding,
                     const std::vector<AVal>& args, Result* result) const {
  AVal res(AVAL_NOBASE);
  bool error_found = false;
  AVal err(AVAL_NOBASE);
  const AVal base(this_binding);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    if ((*it)->IsFunction()) {
      AVal prototype = (*it)->GetProperty(Intern("prototype"));
      // TODO(Constellation) GetProperty result value
      if (res.IsUndefined()) {
        prototype = AVal(heap->MakePrototype(*it));
        (*it)->UpdateProperty(heap, Intern("prototype"), prototype);
      }
      this_binding->UpdatePrototype(heap, prototype);
      const Result ret = interp->EvaluateFunction(*it, base, args, true);

      res |= ret.result();
      // merge result values
      res |= ret.result();
      if (ret.HasException()) {
        error_found = true;
        err |= ret.exception();
      }
    }
  }
  *result = Result(res, err, error_found);
}

AVal AVal::GetPropertyImpl(Symbol name, std::unordered_set<const AObject*>* already_searched) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    if (already_searched->find(*it) == already_searched->end()) {
      already_searched->insert(*it);
      val |= (*it)->GetPropertyImpl(name, already_searched);
    }
  }
  return val;
}

iv::core::UString AVal::ToTypeString() const {
  static const std::array<Pair, 5> kBase = { {
    { 1, "number" },
    { 2, "string" },
    { 4, "boolean" },
    { 8, "undefined" },
    { 16, "null" }
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

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AVAL_H_
