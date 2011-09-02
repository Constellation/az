// adding jsdoc to function's param in runtime
//
// example:
//   TypeExpression { test: function(string) }
//   and Object { test: function(str) { } } is provided,
//   see 2 value shapes,
//   and adding TypeExpression information to Object.test property function
#ifndef AZ_CFA2_PARAM_TYPE_MATCHER_H_
#define AZ_CFA2_PARAM_TYPE_MATCHER_H_
#include <az/cfa2/aval_fwd.h>
#include <az/cfa2/heap.h>
#include <az/jsdoc/type_ast_visitor.h>
namespace az {
namespace cfa2 {

class ParamTypeMatcher : public jsdoc::TypeAstVisitor {
 public:
  ParamTypeMatcher(Heap* heap, const AVal& value)
    : heap_(heap), current_(value) { }

  void Match(jsdoc::TypeExpression* param) {
    assert(param);
    param->Accept(this);
  }

 private:
  // JSDoc TypeExpression Visitor
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
    // not nesting type. END
  }

  void Visit(jsdoc::NullLiteral* node) {
    // not nesting type. END
  }

  void Visit(jsdoc::UndefinedLiteral* node) {
    // not nesting type. END
  }

  void Visit(jsdoc::VoidLiteral* node) {
    // not nesting type. END
  }

  void Visit(jsdoc::UnionType* node) {
  }

  void Visit(jsdoc::ArrayType* node) {
  }

  void Visit(jsdoc::RecordType* node) {
  }

  void Visit(jsdoc::FieldType* node) {
  }

  void Visit(jsdoc::FunctionType* node) {
    for (AVal::ObjectSet::const_iterator it = current_.objects().begin(),
         last = current_.objects().end(); it != last; ++it) {
      AObject* target = *it;
      if (FunctionLiteral* literal = target->AsJSFunction()) {
        heap_->registry()->RegisterFunctionLiteralWithParamType(literal, node);
      }
    }
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

  Heap* heap_;
  AVal current_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_PARAM_TYPE_MATCHER_H_
