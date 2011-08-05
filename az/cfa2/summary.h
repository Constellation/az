#ifndef _AZ_CFA2_SUMMARY_H_
#define _AZ_CFA2_SUMMARY_H_
#include <vector>
#include <iv/detail/memory.h>
#include <iv/detail/unordered_map.h>
#include <iv/detail/tuple.h>
#include <iv/detail/cstdint.h>
#include <az/cfa2/aobject.h>
#include <az/cfa2/answer.h>
#include <az/cfa2/timestamp.h>
namespace az {
namespace cfa2 {

class Summary : private iv::core::Noncopyable<Summary> {
 public:
  // this_binding, arguments and answer tuple
  typedef std::tuple<AVal, std::vector<AVal>, Answer> Entry;
  typedef std::vector<std::shared_ptr<Entry> > Entries;

  Summary(FunctionLiteral* function,
          AObject* obj)
    : function_(function),
      object_(obj),
      candidates_(),
      value_(AVAL_NOBASE),
      timestamp_(kInvalidTimestamp) {
  }

  bool IsExists() const {
    return timestamp_ == kInvalidTimestamp;
  }

  uint64_t timestamp() const {
    return timestamp_;
  }

  AObject* target() const {
    return object_;
  }

  const Entries& candidates() const {
    return candidates_;
  }

 private:
  FunctionLiteral* function_;
  AObject* object_;
  Entries candidates_;
  AVal value_;
  uint64_t timestamp_;
};

typedef std::unordered_map<FunctionLiteral*, std::shared_ptr<Summary> > Summaries;

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_SUMMARY_H_
