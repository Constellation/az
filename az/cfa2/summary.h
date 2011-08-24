#ifndef _AZ_CFA2_SUMMARY_H_
#define _AZ_CFA2_SUMMARY_H_
#include <vector>
#include <iv/detail/memory.h>
#include <iv/detail/unordered_map.h>
#include <iv/detail/tuple.h>
#include <iv/detail/cstdint.h>
#include <az/cfa2/aobject_fwd.h>
#include <az/cfa2/result.h>
#include <az/cfa2/state.h>
namespace az {
namespace cfa2 {

class Summary : private iv::core::Noncopyable<Summary> {
 public:
  // this_binding, arguments and result
  class Entry {
   public:
    Entry(const AVal& this_binding,
          const std::vector<AVal>& args,
          const Result& result)
      : this_binding_(this_binding),
        args_(args),
        result_(result) {
    }

    explicit Entry(const FunctionLiteral* function)
      : this_binding_(AVAL_NOBASE),
        args_(function->params().size(), AVal(AVAL_NOBASE)),
      result_(AVal(AVAL_NOBASE)) {
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

    const Result& result() const {
      return result_;
    }

    void MergeResult(const Result& result) {
      result_.Merge(result);
    }

    void Merge(const AVal& this_binding,
               const std::vector<AVal>& args, const Result& result) {
      MergeThisBinding(this_binding);
      MergeArgs(args);
      result_.Merge(result);
    }

   private:
    AVal this_binding_;
    std::vector<AVal> args_;
    Result result_;
  };

  typedef std::vector<std::shared_ptr<Entry> > Entries;

  Summary(FunctionLiteral* function, AObject* obj)
    : function_(function),
      object_(obj),
      candidates_(),
      value_(AVAL_NOBASE),
      type_(function),
      state_(kInvalidState) {
  }

  bool IsExists() const {
    return state_ != kInvalidState;
  }

  State state() const {
    return state_;
  }

  AObject* target() const {
    return object_;
  }

  const Entries& candidates() const {
    return candidates_;
  }

  void AddCandidate(const AVal& this_binding,
                    const std::vector<AVal>& args, const Result& result) {
    candidates_.push_back(
        std::shared_ptr<Entry>(new Entry(this_binding, args, result)));
  }

  void UpdateCandidates(State state,
                        const AVal& this_binding,
                        const std::vector<AVal>& args, const Result& result) {
    state_ = state;
    candidates_.clear();
    AddCandidate(this_binding, args, result);
  }

  void UpdateType(const AVal& this_binding,
                  const std::vector<AVal>& args, const Result& result) {
    type_.Merge(this_binding, args, result);
  }

  iv::core::UString ToTypeString(Heap* heap) const {
    assert(IsExists());
    iv::core::UString result;
    result.append(type_.result().result().ToTypeString(heap));
    return result;
  }

 private:
  FunctionLiteral* function_;
  AObject* object_;
  Entries candidates_;
  AVal value_;
  Entry type_;
  State state_;
};

// need to be ordered
typedef std::map<FunctionLiteral*, std::shared_ptr<Summary> > Summaries;

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_SUMMARY_H_
