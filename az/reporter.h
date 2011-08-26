#ifndef _AZ_REPORTER_H_
#define _AZ_REPORTER_H_
#include <iv/detail/cinttypes.h>
#include <iv/detail/cstdint.h>
#include <az/basic_reporter.h>
#include <az/structured_source.h>
namespace az {

class Reporter : public BasicReporter {
 public:
  Reporter(const StructuredSource& structured)
    : structured_(structured) {
  }

  void ReportDeadStatement(const Statement& stmt) {
    // report when dead statement is found
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(stmt.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "DEAD CODE STATMENT",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportFunctionStatement(const FunctionStatement& stmt) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(stmt.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "FUNCTION STATEMENT",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportDuplicateDeclaration(const Declaration& decl) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(decl.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "DUPLICATE DECLARATION",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportDeleteToInvalidLHS(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "DELETE TO INVALID LHS",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportDeleteToIdentifier(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "DELETE TO IDENTIFIER",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportDeleteToCallResult(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "DELETE TO CALL RESULT",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportIncrementToCallResult(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "INCREMENT TO CALL RESULT",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportDecrementToCallResult(const UnaryOperation& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "DECREMENT TO CALL RESULT",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportPostfixIncrementToCallResult(const PostfixExpression& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "POSTFIX INCREMENT TO CALL RESULT",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportPostfixDecrementToCallResult(const PostfixExpression& unary) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(unary.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "POSTFIX DECREMENT TO CALL RESULT",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportTypeConflict(const AstNode& node, const AType& lhs, const AType& rhs) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(node.begin_position());
    std::printf("%s %s <=> %s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "TYPE CONFLICT",
                GetTypeName(lhs),
                GetTypeName(rhs),
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportIdentifierAccessToNotObjectType(const IdentifierAccess& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "IDENTIFIER ACCESS TO NOT OBJECT TYPE",
                GetTypeName(type),
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportIndexAccessToNotObjectType(const IndexAccess& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "INDEX ACCESS TO NOT OBJECT TYPE",
                GetTypeName(type),
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportIndexKeyIsNotStringOrNumber(const IndexAccess& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "INDEX KEY IS NOT STRING OR NUMBER",
                GetTypeName(type),
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportCallToNotFunction(const FunctionCall& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "CALL TO NOT FUNCTION",
                GetTypeName(type),
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportConstructToNotFunction(const ConstructorCall& expr, const AType& type) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(expr.begin_position());
    std::printf("%s => %s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "CONSTRUCT TO NOT FUNCTION",
                GetTypeName(type),
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportLookupImplicitGlobalVariable(const Identifier& ident) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(ident.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "LOOKUP IMPLICIT GLOBAL VARIABLE",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportLookupNotDeclaredVariable(const Identifier& ident) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(ident.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "LOOKUP NOT DECLARED VARIABLE",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportAutomaticSemicolonInsertion(std::size_t point) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(point);
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "AUTOMATIC SEMICOLON INSERTION",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportTrailingCommaInObjectLiteral(std::size_t point) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(point);
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "TRAILING COMMA IN OBJECT LITERAL",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportNotProcedure(const Statement& stmt) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(stmt.begin_position());
    std::printf("%s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "NOT PROCEDURE REQUIRE RETURN VALUE",
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

  void ReportSyntaxError(const std::string& str, std::size_t begin_position) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(begin_position);
    std::printf("%s %s " "%"PRIu64 ":" "%"PRIu64 "\n",
                "SYNTAX ERROR",
                str.c_str(),
                static_cast<uint64_t>(pair.first),
                static_cast<uint64_t>(pair.second));
  }

 private:
  const StructuredSource& structured_;
};

}  // namespace az
#endif  // _AZ_BASIC_REPORTER_H_
