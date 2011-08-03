#ifndef _AZ_CFA2_AVAL_FWD_H_
#define _AZ_CFA2_AVAL_FWD_H_
#include <set>
#include <iterator>
#include <algorithm>
#include <iv/noncopyable.h>
#include <iv/ustring.h>
#include <iv/ustringpiece.h>
#include <az/cfa2/fwd.h>
namespace az {
namespace cfa2 {

enum BaseType {
  AVAL_NOBASE = 0,
  AVAL_NUMBER = 1,
  AVAL_STRING = 2,
  AVAL_BOOL = 4,
  AVAL_UNDEFINED = 8,
  AVAL_NULL = 16
};

// abstract value
class AVal {
 public:
  typedef std::set<AObject*> ObjectSet;
  explicit AVal(BaseType type)
    : base_(type),
      objects_() {
  }

  explicit AVal(int type)
    : base_(type),
      objects_() {
  }

  explicit AVal(AObject* obj)
    : base_(AVAL_NOBASE),
      objects_() {
    objects_.insert(obj);
  }

  AVal()
    : base_(AVAL_UNDEFINED),
      objects_() {
  }

  explicit AVal(const iv::core::UStringPiece& str)
    : base_(AVAL_STRING),
      str_(str.begin(), str.end()),
      objects_() {
  }

  bool HasNumber() const {
    return base_ & AVAL_NUMBER;
  }

  bool HasString() const {
    return base_ & AVAL_STRING;
  }

  AVal GetBase() const {
    return AVal(base_);
  }

  AVal BaseToObject() const {
    if (base_ == AVAL_NOBASE) {
      return *this;
    }
    AVal result(AVAL_NOBASE);
    std::copy(objects_.begin(),
              objects_.end(),
              std::inserter(result.objects_, result.objects_.end()));
    if (base_ & AVAL_NUMBER) {
      result.Join(AVAL_NUMBER);
    }
    if (base_ & AVAL_STRING) {
      result.Join(AVAL_STRING);
    }
    if (base_ & AVAL_BOOL) {
      result.Join(AVAL_BOOL);
    }
    return result;
  }

  const iv::core::UString& GetStringValue() const {
    return str_;
  }

  const ObjectSet& objects() const {
    return objects_;
  }

  inline void UpdateProperty(Symbol name, const AVal& val);

  inline AVal GetProperty(Symbol name);

  // join rhs aval to this
  void Join(const AVal& rhs) {
    const int base = base_ | rhs.base_;
    if (base & AVAL_STRING) {
      if (base_ & AVAL_STRING) {
        if ((rhs.base_ & AVAL_STRING) && (str_ != rhs.str_)) {
          // rhs contains AVAL_STRING && lhs.str_ != rhs.str_
          str_.clear();
        }
      } else {
        str_ = rhs.str_;
      }
    }
    base_ = base;
    std::copy(rhs.objects_.begin(),
              rhs.objects_.end(),
              std::inserter(objects_, objects_.end()));
  }

  // join basetype to this
  void Join(BaseType rhs) {
    if (base_ & AVAL_STRING && rhs == AVAL_STRING) {
      str_.clear();
    }
    base_ |= rhs;
  }

  friend AVal operator|(const AVal& lhs, const AVal& rhs) {
    AVal res(lhs);
    res.Join(rhs);
    return res;
  }

  friend bool operator<(const AVal& lhs, const AVal& rhs) {
    if (lhs.base_ > (lhs.base_ & rhs.base_)) {
      // lhs has a base type which rhs doesn't have
      return false;
    }
    if ((lhs.base_ & AVAL_STRING) &&
        !rhs.str_.empty() &&
        rhs.str_ != lhs.str_) {
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
    if (lhs.str_ != rhs.str_) {
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

 private:
  int base_;
  iv::core::UString str_;
  ObjectSet objects_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_AVAL_H_
