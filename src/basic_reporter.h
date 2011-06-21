// reporter base
#ifndef _AZ_BASIC_REPORTER_H_
#define _AZ_BASIC_REPORTER_H_
#include <iv/noncopyable.h>
#include "ast_fwd.h"
namespace az {

class BasicReporter : private iv::core::Noncopyable<BasicReporter> {
 public:
  void ReportDeadStatement(const Statement& stmt) {
  }

  void ReportFunctionStatement(const FunctionStatement& stmt) {
  }

  void ReportDuplicateDeclaration(const Declaration& decl) {
  }

  void ReportAutomaticSemicolonInsertion(const Statement& stmt) {
  }
};

}  // namespace az
#endif  // _AZ_BASIC_REPORTER_H_
