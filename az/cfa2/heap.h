#ifndef _AZ_CFA2_HEAP_H_
#define _AZ_CFA2_HEAP_H_
#include <algorithm>
#include <utility>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/deleter.h>
#include <az/cfa2/builtins_fwd.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/aobject_factory.h>
#include <az/cfa2/summary.h>
#include <az/cfa2/timestamp.h>
namespace az {
namespace cfa2 {

class Heap : private iv::core::Noncopyable<Heap> {
 public:
  friend class Frame;
  typedef std::unordered_set<Binding*> HeapSet;
  Heap()
    : heap_(),
      declared_heap_bindings_(),
      timestamp_(1),
      call_count_(0) {
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
    decls_[node] = obj;
  }

  AObject* GetDeclObject(AstNode* node) const {
    return decls_.find(node)->second;
  }

  void UpdateHeap(Binding* binding, const AVal& val) {
    AVal old(binding->value());
    if (!(val < old)) {
      old.Join(val);
      binding->set_value(old);
      // heap update, so count up timestamp
      binding->set_timestamp(++timestamp_);
    }
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

  void InitPending(AObject* func) {
  }

  void InitSummary(FunctionLiteral* literal, AObject* func) {
    summaries_.insert(
        std::make_pair(
            literal,
            std::shared_ptr<Summary>(new Summary(literal, func))));
  }

  bool FindSummary(AObject* func,
                   const AVal& this_binding,
                   const std::vector<AVal>& args, Answer* result) const {
    Summaries::const_iterator s = summaries_.find(func->function());
    assert(s != summaries_.end());
    if (s->second->timestamp() < timestamp_) {
      // out of date summary
      return false;
    }
    for (Summary::Entries::const_iterator it = s->second->candidates().begin(),
         last = s->second->candidates().end(); it != last; ++it) {
      const Summary::Entry& entry = **it;
      if (entry.this_binding() == this_binding) {
        if (args.size() == entry.args().size()) {
          if (std::equal(args.begin(), args.end(), entry.args().begin())) {
            // fit summary found
            *result = entry.result();
            return true;
          }
        }
      }
    }
    return false;
  }

  void AddSummary(AObject* func,
                  const AVal& this_binding,
                  const std::vector<AVal>& args, const Answer& result) {
    Summaries::iterator s = summaries_.find(func->function());
    assert(s != summaries_.end());
    if (s->second->timestamp() == timestamp_) {
      s->second->AddCandidate(this_binding, args, result);
    } else if (s->second->timestamp() < timestamp_) {
      // old, so clear candidates
      s->second->UpdateCandidates(timestamp_, this_binding, args, result);
    }
    s->second->UpdateType(this_binding, args, result);
  }

  void ShowSummaries() const {
    std::vector<uint16_t> res;
    for (Summaries::const_iterator it = summaries().begin(),
         last = summaries().end(); it != last; ++it) {
      if (const iv::core::Maybe<Identifier> ident = it->first->name()) {
        res.insert(res.end(),
                   ident.Address()->value().begin(),
                   ident.Address()->value().end());
        res.push_back(' ');
      } else {
        static const std::string prefix("<anonymous> ");
        res.insert(res.end(), prefix.begin(), prefix.end());
      }
      const iv::core::UString str(it->second->ToTypeString());
      res.insert(res.end(), str.begin(), str.end());
      res.push_back('\n');
      iv::core::unicode::FPutsUTF16(stdout, res.begin(), res.end());
      res.clear();
    }
  }

  const Summaries& summaries() const {
    return summaries_;
  }

  Summaries& summaries() {
    return summaries_;
  }

  void CountUpCall() {
    ++call_count_;
  }

  void CountUpDepth() {
    ++depth_;
  }

 private:
  HeapSet heap_;
  HeapSet declared_heap_bindings_;
  std::unordered_map<AstNode*, AObject*> decls_;
  std::unordered_map<Binding*, AVal> binding_heap_;
  Summaries summaries_;
  AObjectFactory factory_;
  uint64_t timestamp_;
  uint64_t call_count_;
  uint64_t depth_;

  AVal global_;
  AVal object_prototype_;
  AVal function_prototype_;
  AVal array_function_called_value_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_HEAP_H_
