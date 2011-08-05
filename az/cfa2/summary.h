#ifndef _AZ_CFA2_SUMMARY_H_
#define _AZ_CFA2_SUMMARY_H_
#include <iv/detail/memory.h>
#include <az/cfa2/aobject.h>
#include <az/cfa2/timestamp.h>
namespace az {
namespace cfa2 {

class Summary : private iv::core::Noncopyable<Summary> {
 public:
  Summary(FunctionLiteral* function,
          AObject* obj)
    : function_(function),
      object_(obj),
      value_(AVAL_NOBASE),
      timestamp_(kInvalidTimestamp) {
  }

  bool IsExists() const {
    return timestamp_ == kInvalidTimestamp;
  }

 private:
  FunctionLiteral* function_;
  AObject* object_;
  AVal value_;
  uint64_t timestamp_;
};

typedef std::unordered_map<FunctionLiteral*, std::shared_ptr<Summary> > Summaries;

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_SUMMARY_H_
