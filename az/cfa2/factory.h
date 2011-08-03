// AObject factory
#ifndef _AZ_CFA2_FACTORY_H_
#define _AZ_CFA2_FACTORY_H_
#include <algorithm>
#include <iv/noncopyable.h>
#include <az/deleter.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aobject.h>
namespace az {
namespace cfa2 {

class Factory : private iv::core::Noncopyable<Factory> {
  Factory()
    : created_() {
  }

  AObject* New() {
    AObject* obj = new AObject();
    created_.push_back(obj);
    return obj;
  }

  AObject* New(FunctionLiteral* constructor, AVal proto) {
    AObject* obj = new AObject(constructor, proto);
    created_.push_back(obj);
    return obj;
  }

  ~Factory() {
    std::for_each(created_.begin(), created_.end(), Deleter());
  }

  std::vector<AObject*> created_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_FACTORY_H_
