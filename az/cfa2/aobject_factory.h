// AObject factory
#ifndef AZ_CFA2_AOBJECT_FACTORY_H_
#define AZ_CFA2_AOBJECT_FACTORY_H_
#include <algorithm>
#include <new>
#include <deque>
#include <iv/noncopyable.h>
#include <iv/alloc.h>
#include <az/destructor.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aobject_fwd.h>
namespace az {
namespace cfa2 {

class AObjectFactory : private iv::core::Noncopyable<AObjectFactory> {
 public:
  AObjectFactory()
    : created_objects_() {
  }

  AObject* NewAObject() {
    // AObject* obj = new (&space_) AObject();
    // AObject* obj = new (space_.New(sizeof(AObject))) AObject;
    AObject* obj = new AObject;
    created_objects_.push_back(obj);
    return obj;
  }

  AObject* NewAObject(FunctionLiteral* func, AVal proto) {
    // AObject* obj = new (&space_) AObject(func, proto);
    // AObject* obj = new (space_.New(sizeof(AObject))) AObject(func, proto);
    AObject* obj = new AObject(func, proto);
    created_objects_.push_back(obj);
    return obj;
  }

  AObject* NewAObject(Builtin func, AVal proto) {
    // AObject* obj = new (&space_) AObject(func, proto);
    // AObject* obj = new (space_.New(sizeof(AObject))) AObject(func, proto);
    AObject* obj = new AObject(func, proto);
    created_objects_.push_back(obj);
    return obj;
  }

  AObject* NewAObject(AVal proto) {
    // AObject* obj = new (&space_) AObject(proto);
    // AObject* obj = new (space_.New(sizeof(AObject))) AObject(proto);
    AObject* obj = new AObject(proto);
    created_objects_.push_back(obj);
    return obj;
  }

  ~AObjectFactory() {
    // call destructors
    std::for_each(created_objects_.begin(),
                  created_objects_.end(), Deleter());
  }
  std::deque<AObject*> created_objects_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_AOBJECT_FACTORY_H_
