#ifndef _AZ_CFA2_HEAP_H_
#define _AZ_CFA2_HEAP_H_
#include <algorithm>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/deleter.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/factory.h>
namespace az {
namespace cfa2 {

class Heap : private iv::core::Noncopyable<Heap> {
 public:
  typedef std::unordered_set<Binding*> HeapSet;
  Heap()
    : heap_(),
      declared_heap_bindings_(),
      timestamp_(0) {
    // initialize builtin objects

    // Global
    AObject* global = factory_.NewAObject();
    global_ = AVal(global);
    global->AddProperty(
        Intern("Infinity"),
        AProp(AVal(AVAL_NUMBER), A::N));
    global->AddProperty(
        Intern("NaN"),
        AProp(AVal(AVAL_NUMBER), A::N));
    global->AddProperty(
        Intern("undefined"),
        AProp(AVal(AVAL_UNDEFINED), A::N));

    // Object prototype
    AObject* object_prototype = factory_.NewAObject();
    object_prototype_ = AVal(object_prototype);

    // Object.__proto__
    AObject* object_proto = factory_.NewAObject(object_prototype_);
    function_prototype_ = AVal(object_proto);

    AObject* function_prototype_prototype = factory_.NewAObject(object_prototype_);

    object_prototype->AddProperty(
        Intern("prototype"),
        AProp(AVal(function_prototype_prototype), A::W));
    function_prototype_prototype->AddProperty(
        Intern("constructor"),
        AProp(object_prototype_, A::W));
  }

  ~Heap() {
    std::for_each(heap_.begin(), heap_.end(), Deleter());
  }

  // create new binding object
  Binding* Instantiate(Symbol name) {
    Binding* binding = new Binding(name, Binding::STACK);
    heap_.insert(binding);
    return binding;
  }

  Factory* GetFactory() {
    return &factory_;
  }

  void RecordDeclaredHeapBinding(Binding* binding) {
    declared_heap_bindings_.insert(binding);
  }

  void DeclObject(AstNode* node, AObject* obj) {
    decls_[node] = AVal(obj);
  }

 private:
  HeapSet heap_;
  HeapSet declared_heap_bindings_;
  std::unordered_map<AstNode*, AVal> decls_;
  Factory factory_;
  uint64_t timestamp_;

  AVal global_;
  AVal object_prototype_;
  AVal function_prototype_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_HEAP_H_
