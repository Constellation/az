#ifndef AZ_CFA2_TYPE_AST_VISITOR_H_
#define AZ_CFA2_TYPE_AST_VISITOR_H_
#include <iv/debug.h>
#include <az/ast_fwd.h>
#include <az/jsdoc/type_ast_visitor.h>
namespace az {
namespace cfa2 {

class TypeAstVisitor : public jsdoc::TypeAstVisitor {
 public:
  TypeAstVisitor(AstFactory* factory)
    : factory_(factory),
      result_(NULL) {
  }

  Expression* Get(jsdoc::TypeExpression* expr) {
    // Top Level
    assert(expr);
    expr->Accept(this);
    return result_;
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
    // primitive type check phase
    jsdoc::NameString* str = node->value();
    if (IsEqualIgnoreCase(*str, "string")) {
      std::vector<uint16_t> buffer;
      result_ = factory_->NewStringLiteral(buffer, 0, 0);
      return;
    }
  }

  void Visit(jsdoc::TypeNameWithApplication* node) {
  }

  void Visit(jsdoc::ParametersType* node) {
  }

  void Visit(jsdoc::RestExpression* node) {
  }

  void Visit(jsdoc::PostfixEqualExpression* node) {
  }

  AstFactory* factory_;
  Expression* result_;
};

} }  // namespace az::cfa2
#endif  //AZ_CFA2_TYPE_AST_VISITOR_H_
