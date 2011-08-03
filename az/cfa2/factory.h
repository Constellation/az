// AObject factory
#ifndef _AZ_CFA2_FACTORY_H_
#define _AZ_CFA2_FACTORY_H_
#include <algorithm>
#include <new>
#include <iv/noncopyable.h>
#include <iv/alloc.h>
#include <az/destructor.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aobject.h>
namespace az {
namespace cfa2 {

class Factory : private iv::core::Noncopyable<Factory> {
 public:
  Factory()
    : space_(),
      created_() {
  }

  AObject* New() {
    AObject* obj = new (&space_) AObject();
    created_.push_back(obj);
    return obj;
  }

  AObject* New(FunctionLiteral* constructor, AVal proto) {
    AObject* obj = new (&space_) AObject(constructor, proto);
    created_.push_back(obj);
    return obj;
  }

  ~Factory() {
    // call destructors
    std::for_each(created_.begin(), created_.end(), TypedDestructor<AObject>());
  }

  iv::core::Space<1> space_;
  std::vector<AObject*> created_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_FACTORY_H_
