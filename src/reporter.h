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

  void ReportIncrementToCallResult(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s %lu:%lu\n",
                "INCREMENT TO CALL RESULT",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportDecrementToCallResult(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s %lu:%lu\n",
                "DECREMENT TO CALL RESULT",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportPostfixIncrementToCallResult(const PostfixExpression& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s %lu:%lu\n",
                "POSTFIX INCREMENT TO CALL RESULT",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportPostfixDecrementToCallResult(const PostfixExpression& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s %lu:%lu\n",
                "POSTFIX DECREMENT TO CALL RESULT",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportTypeConflict(const AstNode& node, const AType& lhs, const AType& rhs) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(node.begin_position());
    std::printf("%s %s <=> %s %lu:%lu\n",
                "TYPE CONFLICT",
                GetTypeName(lhs),
                GetTypeName(rhs),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportIdentifierAccessToNotObjectType(const IdentifierAccess& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s %lu:%lu\n",
                "IDENTIFIER ACCESS TO NOT OBJECT TYPE",
                GetTypeName(type),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportIndexAccessToNotObjectType(const IndexAccess& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s %lu:%lu\n",
                "INDEX ACCESS TO NOT OBJECT TYPE",
                GetTypeName(type),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportIndexKeyIsNotStringOrNumber(const IndexAccess& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s %lu:%lu\n",
                "INDEX KEY IS NOT STRING OR NUMBER",
                GetTypeName(type),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportCallToNotFunction(const FunctionCall& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s %lu:%lu\n",
                "CALL TO NOT FUNCTION",
                GetTypeName(type),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportConstructToNotFunction(const ConstructorCall& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s %lu:%lu\n",
                "CONSTRUCT TO NOT FUNCTION",
                GetTypeName(type),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportLookupImplicitGlobalVariable(const Identifier& ident) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(ident.begin_position());
    std::printf("%s %lu:%lu\n",
                "LOOKUP IMPLICIT GLOBAL VARIABLE",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportLookupNotDeclaredVariable(const Identifier& ident) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(ident.begin_position());
    std::printf("%s %lu:%lu\n",
                "LOOKUP NOT DECLARED VARIABLE",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportAutomaticSemicolonInsertion(const Statement& stmt) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(stmt.begin_position());
    std::printf("%s %lu:%lu\n",
                "AUTOMATIC SEMICOLON INSERTION",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportNotProcedure(const Statement& stmt) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(stmt.begin_position());
    std::printf("%s %lu:%lu\n",
                "NOT PROCEDURE: REQUIRE RETURN VALUE",
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

  void ReportSyntaxError(const std::string& str, std::size_t begin_position) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(begin_position);
    std::printf("%s: %s %lu:%lu\n",
                "SYNTAX ERROR",
                str.c_str(),
                static_cast<unsigned long>(pair.first),
                static_cast<unsigned long>(pair.second));
  }

 private:
  StructuredSource structured_;
};

}  // namespace az
#endif  // _AZ_BASIC_REPORTER_H_
