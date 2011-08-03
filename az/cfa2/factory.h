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
      created_objects_(),
      created_values_() {
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

  template<typename T0>
  AVal* NewAVal(const T0& a0) {
    AVal* aval = new (space_.New(sizeof(AVal))) AVal(a0);
    created_values_.push_back(aval);
    return aval;
  }

  template<typename T0, typename T1>
  AVal* NewAVal(const T0& a0, const T1& a1) {
    AVal* aval = new (space_.New(sizeof(AVal))) AVal(a0, a1);
    created_values_.push_back(aval);
    return aval;
  }

  ~Factory() {
    // call destructors
    std::for_each(created_objects_.begin(),
                  created_objects_.end(), TypedDestructor<AObject>());
    std::for_each(created_values_.begin(),
                  created_values_.end(), TypedDestructor<AVal>());
  }

  iv::core::Space<1> space_;
  std::deque<AObject*> created_objects_;
  std::deque<AVal*> created_values_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_FACTORY_H_
