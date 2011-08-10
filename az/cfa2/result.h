#ifndef _AZ_CFA2_RESULT_H_
#define _AZ_CFA2_RESULT_H_
#include <algorithm>
#include <az/cfa2/fwd.h>
#include <az/cfa2/aval_fwd.h>
namespace az {
namespace cfa2 {

class Result {
 public:
  Result()
    : result_(AVAL_NOBASE),
      exception_(AVAL_NOBASE),
      has_exception_(false) {
  }


  explicit Result(AVal result)
    : result_(result),
      exception_(AVAL_NOBASE),
      has_exception_(false) {
  }

  Result(AVal result, AVal exception)
    : result_(result),
      exception_(exception),
      has_exception_(true) {
  }

  Result(AVal result, AVal exception, bool has_exception)
    : result_(result),
      exception_(exception),
      has_exception_(has_exception) {
  }

  const AVal& result() const {
    return result_;
  }

  void set_result(const AVal& res) {
    result_ = res;
  }

  const AVal& exception() const {
    return exception_;
  }

  void set_exception(const AVal& exception) {
    has_exception_ = true;
    exception_ = exception;
  }

  void Merge(const Result& rhs) {
    // return val
    result_.Join(rhs.result_);
    if (rhs.HasException()) {  // error found
      // error val
      exception_.Join(rhs.exception_);
      has_exception_ = true;
    }
  }

  bool HasException() const {
    return has_exception_;
  }

  friend void swap(Result& lhs, Result& rhs) {
    using std::swap;
    swap(lhs.result_, rhs.result_);
    swap(lhs.exception_, rhs.exception_);
    swap(lhs.has_exception_, rhs.has_exception_);
  }

 private:
  AVal result_;
  AVal exception_;
  bool has_exception_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_RESULT_H_