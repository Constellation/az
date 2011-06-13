#ifndef _AZ_REPORTER_H_
#define _AZ_REPORTER_H_
#include "basic_reporter.h"
#include "structured_source.h"
namespace az {

class Reporter : public BasicReporter {
 public:
  template<typename Source>
  Reporter(const Source& src)
    : structured_(src) {
  }

  void ReportDeadStatement(const Statement& stmt) {
    // report when dead statement is found
    std::printf("%s\n", "DEAD CODE");
  }

 private:
  StructuredSource structured_;
};

}  // namespace az
#endif  // _AZ_BASIC_REPORTER_H_
