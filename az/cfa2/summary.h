#ifndef _AZ_CFA2_SUMMARY_H_
#define _AZ_CFA2_SUMMARY_H_
namespace az {
namespace cfa2 {

class Summary {
 public:
 private:
  AVal value_;
  uint64_t timestamp_;
};

class Summarys : private iv::core::Noncopyable<Summarys> {
 public:
 private:
  std::unordered_map<const FunctionLiteral*, std::shared_ptr<Summary> > summarys_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_SUMMARY_H_
