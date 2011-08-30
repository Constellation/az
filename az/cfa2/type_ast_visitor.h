#ifndef _AZ_CFA2_TYPE_AST_VISITOR_H_
#define _AZ_CFA2_TYPE_AST_VISITOR_H_
#include <iv/debug.h>
#include <az/ast_fwd.h>
#include <az/jsdoc/type_ast_visitor.h>
namespace az {
namespace cfa2 {

class TypeAstVisitor : public jsdoc::TypeAstVisitor {
 public:
  Expression* Get(jsdoc::TypeExpression* expr) {
    // Top Level
    assert(expr);
    expr->Accept(this);
    return NULL;
  }

 private:
  void Visit(jsdoc::PrefixQuestionExpression* node) {
  }

  void Visit(jsdoc::PrefixBangExpression* node) {
  }

  void Visit(jsdoc::PostfixQuestionExpression* node) {
  }

  void Visit(jsdoc::PostfixBangExpression* node) {
  }

  void Visit(jsdoc::QuestionLiteral* node) {
  }

  void Visit(jsdoc::StarLiteral* node) {
  }

  void Visit(jsdoc::NullLiteral* node) {
  }

  void Visit(jsdoc::UndefinedLiteral* node) {
  }

  void Visit(jsdoc::VoidLiteral* node) {
  }

  void Visit(jsdoc::UnionType* node) {
  }

  void Visit(jsdoc::ArrayType* node) {
  }

  void Visit(jsdoc::RecordType* node) {
  }

  void Visit(jsdoc::FieldTypeKeyValue* node) {
  }

  void Visit(jsdoc::FunctionType* node) {
  }

  void Visit(jsdoc::NameExpression* node) {
  }

  void Visit(jsdoc::TypeNameWithApplication* node) {
  }

  void Visit(jsdoc::ParametersType* node) {
  }

  void Visit(jsdoc::RestExpression* node) {
  }

  void Visit(jsdoc::PostfixEqualExpression* node) {
  }
};

} }  // namespace az::cfa2
#endif  //_AZ_CFA2_TYPE_AST_VISITOR_H_
