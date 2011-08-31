#ifndef AZ_CFA2_AVAL_FWD_H_
#define AZ_CFA2_AVAL_FWD_H_
#include <set>
#include <algorithm>
#include <iterator>
#include <algorithm>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <iv/ustring.h>
#include <iv/ustringpiece.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/already_searched.h>
namespace az {
namespace cfa2 {

class Completer;

enum BaseType {
  AVAL_NOBASE = 0,
  AVAL_NUMBER = 1,
  AVAL_STRING = 2,
  AVAL_FALSE = 4,
  AVAL_TRUE = 8,
  AVAL_BOOL = 16,
  AVAL_UNDEFINED = 32,
  AVAL_NULL = 64
};

// abstract value
class AVal {
 public:
  typedef std::set<AObject*> ObjectSet;  // need to be ordered
  enum BoolType {
    BOOL_INDETERMINATE = 0,
    BOOL_TRUE = 1,
    BOOL_FALSE = 2
  };
  explicit AVal(BaseType type)
    : base_(type),
      str_(),
      objects_() {
  }

  explicit AVal(int type)
    : base_(type),
      str_(),
      objects_() {
  }

  explicit AVal(AObject* obj)
    : base_(AVAL_NOBASE),
      str_(),
      objects_() {
    objects_.insert(obj);
  }

  AVal()
    : base_(AVAL_UNDEFINED),
      str_(),
      objects_() {
  }

  explicit AVal(const iv::core::UStringPiece& str)
    : base_(AVAL_STRING),
      str_(new iv::core::UString(str.begin(), str.end())),
      objects_() {
  }

  bool HasNumber() const {
    return base_ & AVAL_NUMBER;
  }

  bool HasString() const {
    return base_ & AVAL_STRING;
  }

  bool HasBool() const {
    return base_ & AVAL_BOOL;
  }

  bool IsUndefined() const {
    // undefined only pattern
    return base_ == AVAL_UNDEFINED;
  }

  AVal GetBase() const {
    return AVal(base_);
  }

  AVal ToObject(Heap* heap) const;

  std::shared_ptr<iv::core::UString> GetStringValue() const {
    return str_;
  }

  const ObjectSet& objects() const {
    return objects_;
  }

  void Reset() {
    base_ = AVAL_NOBASE;
    str_.reset();
    objects_.clear();
  }

  inline void UpdateStringProperty(Heap* heap, const AVal& val) const;

  inline void UpdateNumberProperty(Heap* heap, const AVal& val) const;

  inline void UpdateProperty(Heap* heap, Symbol name, const AVal& val) const;

  inline void UpdatePrototype(Heap* heap, const AVal& val) const;

  inline AVal GetProperty(Heap* heap, Symbol name) const;

  inline AVal GetPropertyImpl(Symbol name, AlreadySearched* already_searched) const;

  inline AVal GetStringProperty(Heap* heap) const;

  inline AVal GetStringPropertyImpl(AlreadySearched* already_searched) const;

  inline AVal GetNumberProperty(Heap* heap) const;

  inline AVal GetNumberPropertyImpl(AlreadySearched* already_searched) const;

  inline void Call(Heap* heap,
                   Interpreter* interp,
                   const AVal& this_binding,
                   const std::vector<AVal>& args, Result* result) const;

  inline void Construct(Heap* heap,
                        Interpreter* interp,
                        AObject* this_binding,
                        const std::vector<AVal>& args, Result* result) const;

  inline bool IsTrue() const;

  inline bool IsFalse() const;

  inline void Complete(Heap* heap, Completer* completer) const;

  void ExcludeBase(BaseType base) {
    base_ &= ~base;
  }

  // join rhs aval to this
  void Join(const AVal& rhs) {
    const int base = base_ | rhs.base_;
    if (base & AVAL_STRING) {
      if (base_ & AVAL_STRING) {
        if ((rhs.base_ & AVAL_STRING) &&
            ((str_ && rhs.str_) && (*str_ != *rhs.str_))) {
          // rhs contains AVAL_STRING && lhs.str_ != rhs.str_
          str_.reset();
        }
      } else {
        str_ = rhs.str_;
      }
    }
    base_ = base;
    objects_.insert(rhs.objects_.begin(), rhs.objects_.end());
  }

  // join basetype to this
  void Join(BaseType rhs) {
    if ((base_ & AVAL_STRING) && rhs == AVAL_STRING) {
      str_.reset();
    }
    base_ |= rhs;
  }

  friend AVal operator|(const AVal& lhs, const AVal& rhs) {
    AVal res(lhs);
    res.Join(rhs);
    return res;
  }

  AVal& operator|=(const AVal& rhs) {
    Join(rhs);
    return *this;
  }

  AVal& operator|=(BaseType rhs) {
    Join(rhs);
    return *this;
  }

  // operator< is likely true
  friend bool operator<(const AVal& lhs, const AVal& rhs) {
    if (lhs.base_ > (lhs.base_ & rhs.base_)) {
      // lhs has a base type which rhs doesn't have
      return false;
    }
    if ((lhs.base_ & AVAL_STRING) &&
        (rhs.str_ && lhs.str_) && (*rhs.str_ != *lhs.str_)) {
      // lhs has string & rhs has string & lhs and rhs string is not equal
      return false;
    }
    if (lhs.objects_.empty()) {
      // lhs has no object
      return true;
    }
    if (lhs.objects_.size() > rhs.objects_.size()) {
      // lhs has a lot of objects than rhs
      return false;
    }
    for (ObjectSet::const_iterator it = lhs.objects_.begin(),
         last = lhs.objects_.end(); it != last; ++it) {
      if (rhs.objects_.find(*it) == rhs.objects_.end()) {
        // lhs has an object which rhs doesn't have
        return false;
      }
    }
    return true;
  }

  friend bool operator==(const AVal& lhs, const AVal& rhs) {
    if (lhs.base_ != rhs.base_) {
      return false;
    }
    if ((lhs.str_ && rhs.str_) && (*lhs.str_ != *rhs.str_)) {
      return false;
    }
    if (lhs.objects_.size() != rhs.objects_.size()) {
      return false;
    }
    return std::equal(lhs.objects_.begin(), lhs.objects_.end(), rhs.objects_.begin());
  }

  friend bool operator!=(const AVal& lhs, const AVal& rhs) {
    return !(lhs == rhs);
  }

  friend AVal operator+(const AVal& lhs, const AVal& rhs) {
    // lhs or rhs has object
    if (!lhs.objects_.empty() || !rhs.objects_.empty()) {
      return AVal(AVAL_NUMBER | AVAL_STRING);
    }
    int base = (lhs.base_ | rhs.base_) & AVAL_STRING;
    // not string primitive base is found?
    const int kNotString = AVAL_BOOL | AVAL_NUMBER | AVAL_UNDEFINED | AVAL_NULL;
    if ((lhs.base_ & kNotString) && (rhs.base_ & kNotString)) {
      base |= AVAL_NUMBER;
    }
    return AVal(base);
  }

  friend void swap(AVal& lhs, AVal& rhs) {
    using std::swap;
    swap(lhs.base_, rhs.base_);
    swap(lhs.str_, rhs.str_);
    swap(lhs.objects_, rhs.objects_);
  }

  struct Pair {
    int type;
    const char* str;
  };

  inline iv::core::UString ToTypeString(Heap* heap) const;

 private:
  int base_;
  std::shared_ptr<iv::core::UString> str_;
  ObjectSet objects_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_AVAL_H_
