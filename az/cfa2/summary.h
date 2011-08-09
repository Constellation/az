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
  class Entry {
   public:
    Entry(const AVal& this_binding,
          const std::vector<AVal>& args,
          const Answer& result)
      : this_binding_(this_binding),
        args_(args),
        result_(result) {
    }

    explicit Entry(const FunctionLiteral* function)
      : this_binding_(AVAL_NOBASE),
        args_(function->params().size(), AVal(AVAL_NOBASE)),
        result_(std::make_tuple(AVal(AVAL_NOBASE), false, AVal(AVAL_NOBASE))) {
    }

    const AVal& this_binding() const {
      return this_binding_;
    }

    void MergeThisBinding(const AVal& this_binding) {
      this_binding_.Join(this_binding);
    }

    const std::vector<AVal> args() const {
      return args_;
    }

    void MergeArgs(const std::vector<AVal>& args) {
      std::vector<AVal>::const_iterator args_it = args.begin();
      const std::vector<AVal>::const_iterator args_last = args.end();
      for (std::vector<AVal>::iterator it = args_.begin(),
           last = args_.end(); it != last; ++it) {
        if (args_it != args_last) {
          it->Join(*args_it);
          ++args_it;
        } else {
          it->Join(AVAL_UNDEFINED);
        }
      }
    }

    const Answer& result() const {
      return result_;
    }

    // TODO(Constellation) implement more efficiency Answer
    void MergeResult(const Answer& result) {
      // return val
      std::get<0>(result_) = std::get<0>(result_) | std::get<0>(result);
      if (std::get<1>(result)) {  // error found
        // error val
        std::get<2>(result_) = std::get<2>(result_) | std::get<0>(result);
      }
    }

    void Merge(const AVal& this_binding,
               const std::vector<AVal>& args, const Answer& result) {
      MergeThisBinding(this_binding);
      MergeArgs(args);
      MergeResult(result);
    }

   private:
    AVal this_binding_;
    std::vector<AVal> args_;
    Answer result_;
  };

  typedef std::vector<std::shared_ptr<Entry> > Entries;

  Summary(FunctionLiteral* function,
          AObject* obj)
    : function_(function),
      object_(obj),
      candidates_(),
      value_(AVAL_NOBASE),
      type_(function),
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
    type_.Merge(this_binding, args, result);
  }

  iv::core::UString ToTypeString() const {
    assert(IsExists());
    iv::core::UString result;
    result.append(std::get<0>(type_.result()).ToTypeString());
    return result;
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
