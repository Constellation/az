// reporter base
#ifndef _AZ_BASIC_REPORTER_H_
#define _AZ_BASIC_REPORTER_H_
#include <iv/noncopyable.h>
#include "ast_fwd.h"
namespace az {

class BasicReporter : private iv::core::Noncopyable<BasicReporter> {
  void ReportDeadStatement(const Statement& stmt) {
  }
};

}  // namespace az
#endif  // _AZ_BASIC_REPORTER_H_
