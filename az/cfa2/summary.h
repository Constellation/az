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
    return timestamp_ != kInvalidTimestamp;
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

  void AddCandidate(const AVal& this_binding,
                    const std::vector<AVal>& args, const Answer& result) {
    candidates_.push_back(
        std::shared_ptr<Entry>(new Entry(this_binding, args, result)));
  }

  void UpdateCandidates(uint64_t timestamp,
                        const AVal& this_binding,
                        const std::vector<AVal>& args, const Answer& result) {
    timestamp_ = timestamp;
    candidates_.clear();
    AddCandidate(this_binding, args, result);
  }

  void UpdateType(const AVal& this_binding,
                  const std::vector<AVal>& args, const Answer& result) {
    std::get<0>(type_).Join(this_binding);
    std::vector<AVal>& param = std::get<1>(type_);
    std::vector<AVal>::const_iterator args_it = args.begin();
    const std::vector<AVal>::const_iterator args_last = args.end();
    for (std::vector<AVal>::iterator it = param.begin(),
         last = param.end(); it != last; ++it) {
      if (args_it != args_last) {
        it->Join(*args_it);
        ++args_it;
      } else {
        it->Join(AVAL_UNDEFINED);
      }
    }
    std::get<0>(std::get<2>(type_)).Join(std::get<0>(result));  // return val
    if (std::get<1>(result)) {  // error found
      std::get<2>(std::get<2>(type_)).Join(std::get<2>(result));  // error val
    }
  }

 private:
  FunctionLiteral* function_;
  AObject* object_;
  Entries candidates_;
  AVal value_;
  Entry type_;
  uint64_t timestamp_;
};

typedef std::unordered_map<FunctionLiteral*, std::shared_ptr<Summary> > Summaries;

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_SUMMARY_H_
