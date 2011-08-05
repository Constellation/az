#ifndef _AZ_CFA2_HEAP_H_
#define _AZ_CFA2_HEAP_H_
#include <algorithm>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/deleter.h>
#include <az/cfa2/builtins_fwd.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/aobject_factory.h>
namespace az {
namespace cfa2 {

class Heap : private iv::core::Noncopyable<Heap> {
 public:
  friend class Frame;
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

    object_proto->AddProperty(
        Intern("prototype"),
        AProp(AVal(function_prototype_prototype), A::W));
    function_prototype_prototype->AddProperty(
        Intern("constructor"),
        AProp(object_prototype_, A::W | A::C));

    // Object
    AObject* o = factory_.NewAObject(
        OBJECT_CONSTRUCTOR,
        object_prototype_);
    AVal oav(o);
    global->AddProperty(
        Intern("Object"),
        AProp(oav, A::W | A::C));
    o->AddProperty(
        Intern("prototype"),
        AProp(object_prototype_, A::N));
    object_prototype->AddProperty(
        Intern("constructor"),
        AProp(oav, A::W | A::C));

    // Function
    AObject* f = factory_.NewAObject(object_prototype_);
    AVal fav(f);
    global->AddProperty(
        Intern("Function"),
        AProp(fav, A::W | A::C));
    f->AddProperty(
        Intern("prototype"),
        AProp(object_prototype_, A::N));
    object_proto->AddProperty(
        Intern("constructor"),
        AProp(object_prototype_, A::N));

    // builtin methods

    // Array
    AObject* ap = factory_.NewAObject(object_prototype_);
    AVal apav(ap);

    AObject* anonew = factory_.NewAObject(apav);
    array_function_called_value_ = AVal(anonew);

    AObject* a = factory_.NewAObject(
        ARRAY_CONSTRUCTOR,
        object_prototype_);
    AVal aav(a);
    global->AddProperty(
        Intern("Array"),
        AProp(aav, A::W | A::C));
    a->AddProperty(
        Intern("prototype"),
        AProp(apav, A::N));
    ap->AddProperty(
        Intern("constructor"),
        AProp(aav, A::W | A::C));
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

  AObjectFactory* GetFactory() {
    return &factory_;
  }

  AObject* MakeObject() {
    return factory_.NewAObject(object_prototype_);
  }

  AObject* MakeFunction(FunctionLiteral* function) {
    return factory_.NewAObject(function, function_prototype_);
  }

  void RecordDeclaredHeapBinding(Binding* binding) {
    declared_heap_bindings_.insert(binding);
  }

  void DeclObject(AstNode* node, AObject* obj) {
    decls_[node] = AVal(obj);
  }

  uint64_t timestamp() const {
    return timestamp_;
  }

  AVal GetGlobal() const {
    return global_;
  }

  AVal GetArrayFunctionCalledValue() const {
    return array_function_called_value_;
  }

  void InitSummary(FunctionLiteral* literal, AObject* func) {
  }

  void InitPending(AObject* func) {
  }

 private:
  HeapSet heap_;
  HeapSet declared_heap_bindings_;
  std::unordered_map<AstNode*, AVal> decls_;
  std::unordered_map<Binding*, AVal> binding_heap_;
  AObjectFactory factory_;
  uint64_t timestamp_;

  AVal global_;
  AVal object_prototype_;
  AVal function_prototype_;
  AVal array_function_called_value_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_HEAP_H_
