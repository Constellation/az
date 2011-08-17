#ifndef _AZ_NPAPI_JSON_REPORTER_H_
#define _AZ_NPAPI_JSON_REPORTER_H_
#include <vector>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iv/detail/tuple.h>
namespace az {
namespace npapi {

class JSONReporter : public BasicReporter {
 public:
  typedef std::vector<std::tuple<std::string, std::size_t, std::size_t> > Errors;
  JSONReporter(const StructuredSource& structured)
    : errors_(),
      structured_(structured) {
  }

  std::string Output() {
    std::stringstream ss;
    boost::property_tree::ptree root;
    std::size_t index = 0;
    for (Errors::const_iterator it = errors_.begin(),
         last = errors_.end(); it != last; ++it, ++index) {
      const std::string target = boost::lexical_cast<std::string>(index);
      root.add(target + ".message", std::get<0>(*it));
      root.add(target + ".line", std::get<1>(*it));
      root.add(target + ".col", std::get<2>(*it));
    }
    boost::property_tree::json_parser::write_json(ss, root);
    return ss.str();
  }

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

  // handle this
  void ReportSyntaxError(const std::string& str, std::size_t begin_position) {
    const std::pair<std::size_t, std::size_t> pair =
        structured_.GetLineAndColumn(begin_position);
    errors_.push_back(std::make_tuple(str, pair.first, pair.second));
  }

 private:
  Errors errors_;
  const StructuredSource& structured_;
};

} }  // namespace az::npapi
#endif  // _AZ_NPAPI_JSON_REPORTER_H_
