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
    : created_objects_(),
      space_() {
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

  AObject* NewAObject(Builtin func, AVal proto) {
    AObject* obj = new (&space_) AObject(func, proto);
    created_objects_.push_back(obj);
    return obj;
  }

  AObject* NewAObject(AVal proto) {
    AObject* obj = new (&space_) AObject(proto);
    created_objects_.push_back(obj);
    return obj;
  }

  ~AObjectFactory() {
    // call destructors
    std::for_each(created_objects_.begin(),
                  created_objects_.end(), Destructor());
  }
  std::deque<AObject*> created_objects_;
  iv::core::Space space_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_AOBJECT_FACTORY_H_
