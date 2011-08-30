#ifndef AZ_EMPTY_REPORTER_H_
#define AZ_EMPTY_REPORTER_H_
#include <az/basic_reporter.h>
namespace az {

class EmptyReporter : public BasicReporter {
 public:
  void ReportDeadStatement(const Statement& stmt) {
  }

  void ReportFunctionStatement(const FunctionStatement& stmt) {
  }

  void ReportDuplicateDeclaration(const Declaration& decl) {
  }

  void ReportDeleteToInvalidLHS(const UnaryOperation& unary) {
  }

  void ReportDeleteToIdentifier(const UnaryOperation& unary) {
  }

  void ReportDeleteToCallResult(const UnaryOperation& unary) {
  }

  void ReportIncrementToCallResult(const UnaryOperation& unary) {
  }

  void ReportDecrementToCallResult(const UnaryOperation& unary) {
  }

  void ReportPostfixIncrementToCallResult(const PostfixExpression& unary) {
  }

  void ReportPostfixDecrementToCallResult(const PostfixExpression& unary) {
  }

  void ReportTypeConflict(const AstNode& node, const AType& lhs, const AType& rhs) {
  }

  void ReportIdentifierAccessToNotObjectType(const IdentifierAccess& expr, const AType& type) {
  }

  void ReportIndexAccessToNotObjectType(const IndexAccess& expr, const AType& type) {
  }

  void ReportIndexKeyIsNotStringOrNumber(const IndexAccess& expr, const AType& type) {
  }

  void ReportCallToNotFunction(const FunctionCall& expr, const AType& type) {
  }

  void ReportConstructToNotFunction(const ConstructorCall& expr, const AType& type) {
  }

  void ReportLookupImplicitGlobalVariable(const Identifier& ident) {
  }

  void ReportLookupNotDeclaredVariable(const Identifier& ident) {
  }

  void ReportAutomaticSemicolonInsertion(std::size_t point) {
  }

  void ReportTrailingCommaInObjectLiteral(std::size_t point) {
  }

  void ReportNotProcedure(const Statement& stmt) {
  }

  void ReportSyntaxError(const std::string& str, std::size_t begin_position) {
  }
};

}  // namespace az
#endif  // AZ_EMPTY_REPORTER_H_
