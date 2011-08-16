#ifndef _AZ_CFA2_HEAP_H_
#define _AZ_CFA2_HEAP_H_
#include <algorithm>
#include <utility>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/deleter.h>
#include <az/completer.h>
#include <az/cfa2/builtins_fwd.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/aobject_factory.h>
#include <az/cfa2/summary.h>
#include <az/cfa2/state.h>
#include <az/cfa2/result.h>
namespace az {
namespace cfa2 {

class Heap : private iv::core::Noncopyable<Heap> {
 public:
  friend class Frame;
  typedef std::unordered_set<Binding*> HeapSet;
  typedef std::tuple<AVal, std::vector<AVal>, State, bool> Execution;

  Heap(AstFactory* ast_factory, Completer* completer)
    : heap_(),
      declared_heap_bindings_(),
      decls_(),
      binding_heap_(),
      summaries_(),
      factory_(),
      ast_factory_(ast_factory),
      completer_(completer),
      state_(kInitialState) {

    const std::vector<AVal> empty;
    // initialize builtin objects

    // Global
    AObject* global = factory_.NewAObject();
    global_ = AVal(global);
    // section 15.1.1.1 NaN
    global->AddProperty(
        Intern("NaN"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.1.1.2 Infinity
    global->AddProperty(
        Intern("Infinity"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.1.1.3 undefined
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

    // Global functions
    // section 15.1.2.1 eval(x)
    // TODO(Constellation) fix it

    // section 15.1.2.3 parseInt(string, radix)
    global->AddProperty(
        Intern("parseInt"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.1.2.3 parseFloat(string)
    global->AddProperty(
        Intern("parseFloat"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.1.2.4 isNaN(number)
    global->AddProperty(
        Intern("isNaN"),
        AProp(AVal(factory_.NewAObject(TO_BOOLEAN, function_prototype_)), A::W | A::C));
    // section 15.1.2.5 isFinite(number)
    global->AddProperty(
        Intern("isFinite"),
        AProp(AVal(factory_.NewAObject(TO_BOOLEAN, function_prototype_)), A::W | A::C));
    // section 15.1.3.1 decodeURI(encodedURI)
    global->AddProperty(
        Intern("decodeURI"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.1.3.2 decodeURIComponent(encodedURIComponent)
    global->AddProperty(
        Intern("decodeURIComponent"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.1.3.3 encodeURI(uri)
    global->AddProperty(
        Intern("encodeURI"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.1.3.4 encodeURIComponent(uriComponent)
    global->AddProperty(
        Intern("encodeURIComponent"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section B.2.1 escape(string)
    // this method is deprecated.
    global->AddProperty(
        Intern("escape"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section B.2.2 unescape(string)
    // this method is deprecated.
    global->AddProperty(
        Intern("unescape"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));

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

    // Array
    AObject* ap = factory_.NewAObject(object_prototype_);
    array_prototype_ = AVal(ap);

    AObject* anonew = factory_.NewAObject(array_prototype_);
    array_constructor_result_ = AVal(anonew);

    AObject* a = factory_.NewAObject(
        ARRAY_CONSTRUCTOR,
        function_prototype_);
    AVal aav(a);
    global->AddProperty(
        Intern("Array"),
        AProp(aav, A::W | A::C));
    a->AddProperty(
        Intern("prototype"),
        AProp(array_prototype_, A::N));
    // section 15.4.3.2 Array.isArray(arg)
    // TODO(Constellation) check array
    a->AddProperty(
        Intern("isArray"),
        AProp(AVal(factory_.NewAObject(TO_BOOLEAN, function_prototype_)), A::W | A::C));

    // section 15.5.4.1 Array.prototype.constructor
    ap->AddProperty(
        Intern("constructor"),
        AProp(aav, A::W | A::C));
    // section 15.4.4.2 Array.prototype.toString()
    ap->AddProperty(
        Intern("toString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.4.4.3 Array.prototype.toLocaleString()
    ap->AddProperty(
        Intern("toLocaleString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.4.4.4 Array.prototype.concat([item1[, item2[, ...]]])
    // TODO(Constellation) refine it
    ap->AddProperty(
        Intern("concat"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.5 Array.prototype.join(separator)
    ap->AddProperty(
        Intern("join"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.4.4.6 Array.prototype.pop()
    // TODO(Constellation) refine it
    // section 15.4.4.7 Array.prototype.push([item1[, item2[, ...]]])
    // TODO(Constellation) refine it
    // section 15.4.4.8 Array.prototype.reverse()
    ap->AddProperty(
        Intern("reverse"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.9 Array.prototype.shift()
    // TODO(Constellation) refine it
    // section 15.4.4.10 Array.prototype.slice(start, end)
    ap->AddProperty(
        Intern("slice"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.11 Array.prototype.sort(comparefn)
    ap->AddProperty(
        Intern("sort"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.12
    // Array.prototype.splice(start, deleteCount[, item1[, item2[, ...]]])
    ap->AddProperty(
        Intern("splice"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.13 Array.prototype.unshift([item1[, item2[, ...]]])
    // TODO(Constellation) refine it
    // section 15.4.4.14 Array.prototype.indexOf(searchElement[, fromIndex])
    ap->AddProperty(
        Intern("indexOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.4.4.15 Array.prototype.lastIndexOf(searchElement[, fromIndex])
    ap->AddProperty(
        Intern("lastIndexOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.4.4.16 Array.prototype.every(callbackfn[, thisArg])
    ap->AddProperty(
        Intern("every"),
        AProp(AVal(factory_.NewAObject(TO_BOOLEAN, function_prototype_)), A::W | A::C));
    // section 15.4.4.17 Array.prototype.some(callbackfn[, thisArg])
    ap->AddProperty(
        Intern("some"),
        AProp(AVal(factory_.NewAObject(TO_BOOLEAN, function_prototype_)), A::W | A::C));
    // section 15.4.4.18 Array.prototype.forEach(callbackfn[, thisArg])
    // TODO(Constellation) refine it
    ap->AddProperty(
        Intern("forEach"),
        AProp(AVal(factory_.NewAObject(TO_UNDEFINED, function_prototype_)), A::W | A::C));
    // section 15.4.4.19 Array.prototype.map(callbackfn[, thisArg])
    // TODO(Constellation) refine it
    ap->AddProperty(
        Intern("map"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.20 Array.prototype.filter(callbackfn[, thisArg])
    // TODO(Constellation) refine it
    ap->AddProperty(
        Intern("filter"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.21 Array.prototype.reduce(callbackfn[, initialValue])
    // TODO(Constellation) refine it
    ap->AddProperty(
        Intern("reduce"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));
    // section 15.4.4.22 Array.prototype.reduceRight(callbackfn[, initialValue])
    // TODO(Constellation) refine it
    ap->AddProperty(
        Intern("reduceRight"),
        AProp(AVal(factory_.NewAObject(TO_THIS, function_prototype_)), A::W | A::C));

    // section 15.5 String
    AObject* sp = factory_.NewAObject(object_prototype_);
    string_prototype_ = AVal(sp);
    AObject* s = factory_.NewAObject(
        STRING_CONSTRUCTOR,
        function_prototype_);
    AVal sav(s);
    global->AddProperty(
        Intern("String"),
        AProp(sav, A::W | A::C));

    // section 15.5.3.1 String.prototype
    s->AddProperty(
        Intern("prototype"),
        AProp(string_prototype_, A::N));
    // section 15.5.3.2 String.fromCharCode([char0 [, char1[, ...]]])
    s->AddProperty(
        Intern("fromCharCode"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));

    // section 15.5.4.1 String.prototype.constructor
    sp->AddProperty(
        Intern("constructor"),
        AProp(sav, A::W | A::C));
    // section 15.5.4.2 String.prototype.toString()
    sp->AddProperty(
        Intern("toString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.3 String.prototype.valueOf()
    sp->AddProperty(
        Intern("valueOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.5.4.4 String.prototype.charAt(pos)
    sp->AddProperty(
        Intern("charAt"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.5 String.prototype.charCodeAt(pos)
    sp->AddProperty(
        Intern("charCodeAt"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.5.4.6 String.prototype.concat([string1[, string2[, ...]]])
    sp->AddProperty(
        Intern("concat"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.7 String.prototype.indexOf(searchString, position)
    sp->AddProperty(
        Intern("indexOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.5.4.8 String.prototype.lastIndexOf(searchString, position)
    sp->AddProperty(
        Intern("lastIndexOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.5.4.9 String.prototype.localeCompare(that)
    sp->AddProperty(
        Intern("localeCompare"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.5.4.10 String.prototype.match(regexp)
    AObject* maresult = MakeObject();
    string_match_result_ = AVal(maresult);
    ARRAY_CONSTRUCTOR(this, string_match_result_, empty, true);
    // TODO(Constellation) num prop
    sp->AddProperty(
        Intern("match"),
        AProp(AVal(factory_.NewAObject(StringMatch, function_prototype_)), A::W | A::C));
    // section 15.5.4.11 String.prototype.replace(searchValue, replaceValue)
    sp->AddProperty(
        Intern("replace"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.12 String.prototype.search(regexp)
    sp->AddProperty(
        Intern("search"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.5.4.13 String.prototype.slice(start, end)
    sp->AddProperty(
        Intern("slice"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.14 String.prototype.split(separator, limit)
    AObject* spresult = MakeObject();
    string_split_result_ = AVal(spresult);
    ARRAY_CONSTRUCTOR(this, string_split_result_, empty, true);
    // TODO(Constellation) num prop
    sp->AddProperty(
        Intern("split"),
        AProp(AVal(factory_.NewAObject(StringSplit, function_prototype_)), A::W | A::C));
    // section 15.5.4.15 String.prototype.substring(start, end)
    sp->AddProperty(
        Intern("substring"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.16 String.prototype.toLowerCase()
    sp->AddProperty(
        Intern("toLowerCase"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.17 String.prototype.toLocaleLowerCase()
    sp->AddProperty(
        Intern("toLocaleLowerCase"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.18 String.prototype.toUpperCase()
    sp->AddProperty(
        Intern("toUpperCase"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.19 String.prototype.toLocaleUpperCase()
    sp->AddProperty(
        Intern("toLocaleUpperCase"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.5.4.20 String.prototype.trim()
    sp->AddProperty(
        Intern("trim"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section B.2.3 String.prototype.substr(start, length)
    // this method is deprecated.
    sp->AddProperty(
        Intern("substr"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    string_object_ = AVal(factory_.NewAObject(string_prototype_));

    // section 15.6 Boolean
    AObject* bp = factory_.NewAObject(object_prototype_);
    boolean_prototype_ = AVal(bp);
    AObject* b = factory_.NewAObject(
        BOOLEAN_CONSTRUCTOR,
        function_prototype_);
    AVal bav(b);
    global->AddProperty(
        Intern("Boolean"),
        AProp(bav, A::W | A::C));

    // section 15.6.3.1 Boolean.prototype
    b->AddProperty(
        Intern("prototype"),
        AProp(boolean_prototype_, A::N));

    // section 15.6.4.1 Boolean.prototype.constructor
    bp->AddProperty(
        Intern("constructor"),
        AProp(bav, A::W | A::C));
    // section 15.6.4.2 Boolean.prototype.toString()
    bp->AddProperty(
        Intern("toString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.6.4.3 Boolean.prototype.valueOf()
    bp->AddProperty(
        Intern("valueOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    boolean_object_ = AVal(factory_.NewAObject(boolean_prototype_));

    // section 15.7 Number
    AObject* np = factory_.NewAObject(object_prototype_);
    number_prototype_ = AVal(np);
    AObject* n = factory_.NewAObject(
        NUMBER_CONSTRUCTOR,
        function_prototype_);
    AVal nav(n);
    global->AddProperty(
        Intern("Number"),
        AProp(nav, A::W | A::C));

    // section 15.7.3.1 Number.prototype
    n->AddProperty(
        Intern("prototype"),
        AProp(number_prototype_, A::N));
    // section 15.7.3.2 Number.MAX_VALUE
    n->AddProperty(
        Intern("MAX_VALUE"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.7.3.3 Number.MIN_VALUE
    n->AddProperty(
        Intern("MIN_VALUE"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.7.3.4 Number.NaN
    n->AddProperty(
        Intern("NaN"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.7.3.5 Number.NEGATIVE_INFINITY
    n->AddProperty(
        Intern("NEGATIVE_INFINITY"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.7.3.6 Number.POSITIVE_INFINITY
    n->AddProperty(
        Intern("POSITIVE_INFINITY"),
        AProp(AVal(AVAL_NUMBER), A::N));

    // section 15.7.4.1 Number.prototype.constructor
    np->AddProperty(
        Intern("constructor"),
        AProp(nav, A::W | A::C));
    // section 15.7.4.2 Number.prototype.toString([radix])
    np->AddProperty(
        Intern("toString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.7.4.3 Number.prototype.toLocaleString()
    np->AddProperty(
        Intern("toLocaleString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.7.4.4 Number.prototype.valueOf()
    np->AddProperty(
        Intern("valueOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.7.4.5 Number.prototype.toFixed(fractionDigits)
    np->AddProperty(
        Intern("toFixed"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.7.4.6 Number.prototype.toExponential(fractionDigits)
    np->AddProperty(
        Intern("toExponential"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.7.4.7 Number.prototype.toPrecision(precision)
    np->AddProperty(
        Intern("toPrecision"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    number_object_ = AVal(factory_.NewAObject(number_prototype_));

    // section 15.8 Math
    AObject* m = factory_.NewAObject(object_prototype_);
    AVal mav(m);
    global->AddProperty(
        Intern("Math"),
        AProp(mav, A::W | A::C));
    // section 15.8.1.1 E
    m->AddProperty(
        Intern("E"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.1.2 LN10
    m->AddProperty(
        Intern("LN10"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.1.3 LN2
    m->AddProperty(
        Intern("LN2"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.1.4 LOG2E
    m->AddProperty(
        Intern("LOG2E"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.1.5 LOG10E
    m->AddProperty(
        Intern("LOG10E"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.1.6 PI
    m->AddProperty(
        Intern("PI"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.1.7 SQRT1_2
    m->AddProperty(
        Intern("SQRT1_2"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.1.8 SQRT2
    m->AddProperty(
        Intern("SQRT2"),
        AProp(AVal(AVAL_NUMBER), A::N));
    // section 15.8.2.1 abs(x)
    m->AddProperty(
        Intern("abs"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.2 acos(x)
    m->AddProperty(
        Intern("acos"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.3 asin(x)
    m->AddProperty(
        Intern("asin"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.4 atan(x)
    m->AddProperty(
        Intern("atan"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.5 atan2(y, x)
    m->AddProperty(
        Intern("atan2"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.6 ceil(x)
    m->AddProperty(
        Intern("ceil"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.7 cos(x)
    m->AddProperty(
        Intern("cos"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.8 exp(x)
    m->AddProperty(
        Intern("exp"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.9 floor(x)
    m->AddProperty(
        Intern("floor"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.10 log(x)
    m->AddProperty(
        Intern("log"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.11 max([value1[, value2[, ... ]]])
    m->AddProperty(
        Intern("max"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.12 min([value1[, value2[, ... ]]])
    m->AddProperty(
        Intern("min"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.13 pow(x, y)
    m->AddProperty(
        Intern("pow"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.14 random()
    m->AddProperty(
        Intern("random"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.15 round(x)
    m->AddProperty(
        Intern("round"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.16 sin(x)
    m->AddProperty(
        Intern("sin"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.17 sqrt(x)
    m->AddProperty(
        Intern("sqrt"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.8.2.18 tan(x)
    m->AddProperty(
        Intern("tan"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));

    // section 15.9 Date
    AObject* dp = factory_.NewAObject(object_prototype_);
    date_prototype_ = AVal(dp);
    AObject* d = factory_.NewAObject(
        DATE_CONSTRUCTOR,
        function_prototype_);
    AVal dav(d);
    global->AddProperty(
        Intern("Date"),
        AProp(dav, A::W | A::C));

    // section 15.9.4.1 Date.prototype
    d->AddProperty(
        Intern("prototype"),
        AProp(date_prototype_, A::N));
    // section 15.9.4.2 Date.parse(string)
    d->AddProperty(
        Intern("parse"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.4.3 Date.UTC()
    d->AddProperty(
        Intern("UTC"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.4.4 Date.now()
    d->AddProperty(
        Intern("now"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.1 Date.prototype.constructor
    dp->AddProperty(
        Intern("constructor"),
        AProp(dav, A::W | A::C));
    // section 15.9.5.2 Date.prototype.toString()
    dp->AddProperty(
        Intern("toString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.9.5.3 Date.prototype.toDateString()
    dp->AddProperty(
        Intern("toDateString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.9.5.4 Date.prototype.toTimeString()
    dp->AddProperty(
        Intern("toTimeString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.9.5.5 Date.prototype.toLocaleString()
    dp->AddProperty(
        Intern("toLocaleString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.9.5.6 Date.prototype.toLocaleDateString()
    dp->AddProperty(
        Intern("toLocaleDateString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.9.5.7 Date.prototype.toLocaleTimeString()
    dp->AddProperty(
        Intern("toLocaleTimeString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.9.5.8 Date.prototype.valueOf()
    dp->AddProperty(
        Intern("valueOf"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.9 Date.prototype.getTime()
    dp->AddProperty(
        Intern("getTime"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.10 Date.prototype.getFullYear()
    dp->AddProperty(
        Intern("getFullYear"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.11 Date.prototype.getUTCFullYear()
    dp->AddProperty(
        Intern("getUTCFullYear"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.12 Date.prototype.getMonth()
    dp->AddProperty(
        Intern("getMonth"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.13 Date.prototype.getUTCMonth()
    dp->AddProperty(
        Intern("getUTCMonth"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.14 Date.prototype.getDate()
    dp->AddProperty(
        Intern("getDate"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.15 Date.prototype.getUTCDate()
    dp->AddProperty(
        Intern("getUTCDate"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.16 Date.prototype.getDay()
    dp->AddProperty(
        Intern("getDay"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.17 Date.prototype.getUTCDay()
    dp->AddProperty(
        Intern("getUTCDay"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.18 Date.prototype.getHours()
    dp->AddProperty(
        Intern("getHours"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.19 Date.prototype.getUTCHours()
    dp->AddProperty(
        Intern("getUTCHours"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.20 Date.prototype.getMinutes()
    dp->AddProperty(
        Intern("getMinutes"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.21 Date.prototype.getUTCMinutes()
    dp->AddProperty(
        Intern("getUTCMinutes"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.22 Date.prototype.getSeconds()
    dp->AddProperty(
        Intern("getSeconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.23 Date.prototype.getUTCSeconds()
    dp->AddProperty(
        Intern("getUTCSeconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.24 Date.prototype.getMilliseconds()
    dp->AddProperty(
        Intern("getMilliseconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.25 Date.prototype.getUTCMilliseconds()
    dp->AddProperty(
        Intern("getUTCMilliseconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.26 Date.prototype.getTimezoneOffset()
    dp->AddProperty(
        Intern("getTimezoneOffset"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.27 Date.prototype.setTime(time)
    dp->AddProperty(
        Intern("setTime"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.28 Date.prototype.setMilliseconds(ms)
    dp->AddProperty(
        Intern("setMilliseconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.29 Date.prototype.setUTCMilliseconds(ms)
    dp->AddProperty(
        Intern("setUTCMilliseconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.30 Date.prototype.setSeconds(sec[, ms])
    dp->AddProperty(
        Intern("setSeconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.31 Date.prototype.setUTCSeconds(sec[, ms])
    dp->AddProperty(
        Intern("setUTCSeconds"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.32 Date.prototype.setMinutes(min[, sec[, ms]])
    dp->AddProperty(
        Intern("setMinutes"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.33 Date.prototype.setUTCMinutes(min[, sec[, ms]])
    dp->AddProperty(
        Intern("setUTCMinutes"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.34 Date.prototype.setHours(hour[, min[, sec[, ms]])
    dp->AddProperty(
        Intern("setHours"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.35 Date.prototype.setUTCHours(hour[, min[, sec[, ms]])
    dp->AddProperty(
        Intern("setUTCHours"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.36 Date.prototype.setDate(date)
    dp->AddProperty(
        Intern("setDate"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.37 Date.prototype.setUTCDate(date)
    dp->AddProperty(
        Intern("setUTCDate"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.38 Date.prototype.setMonth(month[, date])
    dp->AddProperty(
        Intern("setMonth"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.39 Date.prototype.setUTCMonth(month[, date])
    dp->AddProperty(
        Intern("setUTCMonth"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.40 Date.prototype.setFullYear(year[, month[, date]])
    dp->AddProperty(
        Intern("setFullYear"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.41 Date.prototype.setUTCFullYear(year[, month[, date]])
    dp->AddProperty(
        Intern("setUTCFullYear"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section 15.9.5.42 Date.prototype.toUTCString()
    // section B.2.6 Date.prototype.toGMTString()
    //   these two function is same object (defined in specification)
    const AVal toUTCString(factory_.NewAObject(TO_STRING, function_prototype_));
    dp->AddProperty(
        Intern("toUTCString"),
        AProp(toUTCString, A::W | A::C));
    dp->AddProperty(
        Intern("toGMTString"),
        AProp(toUTCString, A::W | A::C));
    // section 15.9.5.43 Date.prototype.toISOString()
    dp->AddProperty(
        Intern("toISOString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section 15.9.5.44 Date.prototype.toJSON()
    dp->AddProperty(
        Intern("toJSON"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
    // section B.2.4 Date.prototype.getYear()
    // this method is deprecated.
    dp->AddProperty(
        Intern("getYear"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));
    // section B.2.5 Date.prototype.setYear(year)
    // this method is deprecated.
    dp->AddProperty(
        Intern("setYear"),
        AProp(AVal(factory_.NewAObject(TO_NUMBER, function_prototype_)), A::W | A::C));

    // section 15.10 RegExp
    AObject* rp = factory_.NewAObject(object_prototype_);
    regexp_prototype_ = AVal(rp);
    AObject* r = factory_.NewAObject(
        REGEXP_CONSTRUCTOR,
        function_prototype_);
    AVal rav(r);
    global->AddProperty(
        Intern("RegExp"),
        AProp(rav, A::W | A::C));

    // section 15.10.5.1 RegExp.prototype
    r->AddProperty(
        Intern("prototype"),
        AProp(regexp_prototype_, A::N));

    // section 15.10.6.1 RegExp.prototype.constructor
    rp->AddProperty(
        Intern("constructor"),
        AProp(rav, A::W | A::C));
    // section 15.10.6.2 RegExp.prototype.exec(string)
    AObject* exresult = MakeObject();
    regexp_exec_result_ = AVal(exresult);
    ARRAY_CONSTRUCTOR(this, regexp_exec_result_, empty, true);
    regexp_exec_result_.UpdateProperty(this, Intern("index"), AVal(AVAL_NUMBER));
    regexp_exec_result_.UpdateProperty(this, Intern("input"), AVal(AVAL_STRING));
    rp->AddProperty(
        Intern("exec"),
        AProp(AVal(factory_.NewAObject(RegExpExec, function_prototype_)), A::W | A::C));
    // section 15.10.6.3 RegExp.prototype.test(string)
    rp->AddProperty(
        Intern("test"),
        AProp(AVal(factory_.NewAObject(TO_BOOLEAN, function_prototype_)), A::W | A::C));
    // section 15.10.6.4 RegExp.prototype.toString()
    rp->AddProperty(
        Intern("toString"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));

    // section 15.12 JSON
    AObject* json = factory_.NewAObject(object_prototype_);
    AVal jsonav(json);
    // section 15.12.2 parse(text[, reviver])
    AObject* paresult = MakeObject();
    json_parse_result_ = AVal(paresult);
    json->AddProperty(
        Intern("parse"),
        AProp(AVal(factory_.NewAObject(JSONParse, function_prototype_)), A::W | A::C));
    // section 15.12.3 stringify(value[, replacer[, space]])
    json->AddProperty(
        Intern("stringify"),
        AProp(AVal(factory_.NewAObject(TO_STRING, function_prototype_)), A::W | A::C));
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

  AObject* MakePrototype(AObject* constructor) {
    AObject* obj = MakeObject();
    obj->AddProperty(
        Intern("constructor"),
        AProp(AVal(constructor), A::W | A::C));
    return obj;
  }

  AObject* MakeFunction(FunctionLiteral* function) {
    return factory_.NewAObject(function, function_prototype_);
  }

  void RecordDeclaredHeapBinding(Binding* binding) {
    declared_heap_bindings_.insert(binding);
  }

  ExpressionStatement* NewWrappedStatement(Expression* expr) {
    return ast_factory_->NewExpressionStatement(expr, expr->end_position());
  }

  // global
  bool IsDeclaredHeapBinding(Binding* binding) const {
    return declared_heap_bindings_.find(binding) != declared_heap_bindings_.end();
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
      // heap update, so count up state
      UpdateState();
      binding->set_state(state());
    }
  }

  State state() const {
    return state_;
  }

  AVal GetGlobal() const {
    return global_;
  }

  void InitSummary(FunctionLiteral* literal, AObject* func) {
    summaries_.insert(
        std::make_pair(
            literal,
            std::shared_ptr<Summary>(new Summary(literal, func))));
  }

  bool FindSummary(AObject* func,
                   const AVal& this_binding,
                   const std::vector<AVal>& args, Result* result) const {
    Summaries::const_iterator s = summaries_.find(func->function());
    assert(s != summaries_.end());
    if (s->second->state() < state_) {
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
                  State state,
                  const AVal& this_binding,
                  const std::vector<AVal>& args, const Result& result) {
    Summaries::iterator s = summaries_.find(func->function());
    assert(s != summaries_.end());
    if (s->second->state() == state) {
      s->second->AddCandidate(this_binding, args, result);
    } else if (s->second->state() < state) {
      // old, so clear candidates
      s->second->UpdateCandidates(state, this_binding, args, result);
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

  void UpdateState() {
    ++state_;
  }

  void InitWaitingResults(FunctionLiteral* lit) {
    assert(waiting_result_.find(lit) == waiting_result_.end());
    waiting_result_.insert(
        std::make_pair(lit, std::shared_ptr<ExecutionQueue>(new ExecutionQueue())));
  }

  std::shared_ptr<Execution> AddWaitingResults(
      FunctionLiteral* lit,
      const AVal& this_binding,
      const std::vector<AVal> args,
      bool last) {
    WaitingMap::const_iterator it = waiting_result_.find(lit);
    assert(it != waiting_result_.end());
    std::shared_ptr<Execution> waiting(
        new Execution(this_binding, args, state_, last));
    it->second->push_back(waiting);
    return waiting;
  }

  std::shared_ptr<Execution> RemoveWaitingResults(FunctionLiteral* lit) {
    WaitingMap::iterator it = waiting_result_.find(lit);
    assert(it != waiting_result_.end());
    assert(it->second);
    assert(!it->second->empty());
    std::shared_ptr<Execution> last = it->second->back();
    it->second->pop_back();
    return last;
  }

  std::shared_ptr<Execution> SearchWaitingResults(FunctionLiteral* lit,
                                                  const AVal& this_binding,
                                                  const std::vector<AVal>& args) const {
    WaitingMap::const_iterator it = waiting_result_.find(lit);
    assert(it != waiting_result_.end());
    for (ExecutionQueue::const_reverse_iterator qit = it->second->rbegin(),
         last = it->second->rend(); qit != last; ++qit) {
      if (std::get<0>(**qit) == this_binding) {
        if (std::get<1>(**qit).size() == args.size()) {
          if (std::equal(args.begin(),
                         args.end(),
                         std::get<1>(**qit).begin())) {
            return *qit;
          }
        }
      }
    }
    return std::shared_ptr<Execution>();
  }

  // prototype getters

  const AVal& GetArrayPrototype() const {
    return array_prototype_;
  }

  const AVal& GetStringPrototype() const {
    return string_prototype_;
  }

  const AVal& GetBooleanPrototype() const {
    return boolean_prototype_;
  }

  const AVal& GetNumberPrototype() const {
    return number_prototype_;
  }

  const AVal& GetDatePrototype() const {
    return date_prototype_;
  }

  const AVal& GetRegExpPrototype() const {
    return regexp_prototype_;
  }


  // results
  const AVal& GetArrayConstructorResult() const {
    return array_constructor_result_;
  }

  const AVal& GetStringMatchResult() const {
    return string_match_result_;
  }

  const AVal& GetStringSplitResult() const {
    return string_split_result_;
  }

  const AVal& GetRegExpExecResult() const {
    return regexp_exec_result_;
  }

  const AVal& GetJSONParseResult() const {
    return json_parse_result_;
  }

  const AVal& GetStringObject() const {
    return string_object_;
  }

  const AVal& GetBooleanObject() const {
    return boolean_object_;
  }

  const AVal& GetNumberObject() const {
    return number_object_;
  }

 private:
  HeapSet heap_;
  HeapSet declared_heap_bindings_;
  std::unordered_map<AstNode*, AObject*> decls_;
  std::unordered_map<Binding*, AVal> binding_heap_;
  Summaries summaries_;
  AObjectFactory factory_;
  AstFactory* ast_factory_;
  Completer* completer_;
  State state_;

  AVal global_;
  AVal object_prototype_;
  AVal function_prototype_;
  AVal array_prototype_;
  AVal string_prototype_;
  AVal boolean_prototype_;
  AVal number_prototype_;
  AVal date_prototype_;
  AVal regexp_prototype_;

  // results
  AVal array_constructor_result_;
  AVal string_match_result_;
  AVal string_split_result_;
  AVal regexp_exec_result_;
  AVal json_parse_result_;

  // primitive object values
  AVal string_object_;
  AVal boolean_object_;
  AVal number_object_;

  typedef std::deque<std::shared_ptr<Execution> > ExecutionQueue;
  typedef std::unordered_map<const FunctionLiteral*, std::shared_ptr<ExecutionQueue> > WaitingMap;
  WaitingMap waiting_result_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_HEAP_H_
