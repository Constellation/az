// AObject factory
#ifndef _AZ_CFA2_FACTORY_H_
#define _AZ_CFA2_FACTORY_H_
#include <algorithm>
#include <new>
#include <deque>
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
      created_objects_() {
  }

  AObject* NewAObject() {
    AObject* obj = new (&space_) AObject();
    created_objects_.push_back(obj);
    return obj;
  }

  AObject* NewAObject(FunctionLiteral* func, AVal proto) {
    AObject* obj = new (&space_) AObject(func, proto);
    created_objects_.push_back(obj);
    return obj;
  }

  AObject* NewAObject(AVal proto) {
    AObject* obj = new (&space_) AObject(proto);
    created_objects_.push_back(obj);
    return obj;
  }

  ~Factory() {
    // call destructors
    std::for_each(created_objects_.begin(),
                  created_objects_.end(), TypedDestructor<AObject>());
  }

  iv::core::Space<1> space_;
  std::deque<AObject*> created_objects_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_FACTORY_H_
