// reporter base
#ifndef AZ_BASIC_REPORTER_H_
#define AZ_BASIC_REPORTER_H_
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
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
#endif  // AZ_BASIC_REPORTER_H_
