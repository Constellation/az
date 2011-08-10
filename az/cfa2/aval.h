#ifndef _AZ_CFA2_AVAL_H_
#define _AZ_CFA2_AVAL_H_
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/aobject.h>
namespace az {
namespace cfa2 {


void AVal::UpdateProperty(Heap* heap, Symbol name, const AVal& val) const {
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    (*it)->UpdateProperty(heap, name, val);
  }
}

AVal AVal::GetProperty(Symbol name) const {
  AVal val(AVAL_NOBASE);
  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    // get object property and merge it
    val.Join((*it)->GetProperty(name));
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

  for (ObjectSet::const_iterator it = objects_.begin(),
       last = objects_.end(); it != last; ++it) {
    types.insert((*it)->ToTypeString());
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
