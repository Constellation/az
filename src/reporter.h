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
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(stmt.begin_position());
    std::printf("%s %lu:%lu\n",
                "DEAD CODE STATMENT",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportFunctionStatement(const FunctionStatement& stmt) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(stmt.begin_position());
    std::printf("%s %lu:%lu\n",
                "FUNCTION STATEMENT",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportDuplicateDeclaration(const Declaration& decl) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(decl.begin_position());
    std::printf("%s %lu:%lu\n",
                "DUPLICATE DECLARATION",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportDeleteToInvalidLHS(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s %lu:%lu\n",
                "DELETE TO INVALID LHS",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportDeleteToIdentifier(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s %lu:%lu\n",
                "DELETE TO IDENTIFIER",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportDeleteToCallResult(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s %lu:%lu\n",
                "DELETE TO CALL RESULT",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportTypeConflict(const AstNode& node, JSType lhs, JSType rhs) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(node.begin_position());
    std::printf("%s %s <=> %s %lu:%lu\n",
                "TYPE CONFLICT",
                GetTypeName(lhs),
                GetTypeName(rhs),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

 private:
  StructuredSource structured_;
};

}  // namespace az
#endif  // _AZ_BASIC_REPORTER_H_
