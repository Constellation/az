#ifndef AZ_CFA2_TYPE_INTERPRETER_FWD_H_
#define AZ_CFA2_TYPE_INTERPRETER_FWD_H_
#include <iv/noncopyable.h>
#include <iv/detail/unordered_set.h>
#include <az/cfa2/heap.h>
#include <az/jsdoc/type_ast.h>
#include <az/jsdoc/type_ast_visitor.h>
namespace az {
namespace cfa2 {

class TypeInterpreter
  : private iv::core::Noncopyable<TypeInterpreter>,
    public jsdoc::TypeAstVisitor {
 public:
  static AVal Interpret(Heap* heap, jsdoc::TypeExpression* expr) {
    TypeInterpreter interp(heap);
    expr->Accept(&interp);
    return interp.result_;
  }

  struct UStringPieceHash {
    std::size_t operator()(iv::core::UStringPiece target) const {
      return iv::core::StringToHash(target);
    }
  };

  struct UStringPieceEquals {
    bool operator()(iv::core::UStringPiece lhs, iv::core::UStringPiece rhs) const {
      return lhs == rhs;
    }
  };

 private:
  explicit TypeInterpreter(Heap* heap)
    : heap_(heap),
      result_(AVAL_NOBASE),
      lookuped_() { }

  // JSDoc TypeExpression Visitor
  inline void Visit(jsdoc::PrefixQuestionExpression* node);
  inline void Visit(jsdoc::PrefixBangExpression* node);
  inline void Visit(jsdoc::PostfixQuestionExpression* node);
  inline void Visit(jsdoc::PostfixBangExpression* node);
  inline void Visit(jsdoc::QuestionLiteral* node);
  inline void Visit(jsdoc::StarLiteral* node);
  inline void Visit(jsdoc::NullLiteral* node);
  inline void Visit(jsdoc::UndefinedLiteral* node);
  inline void Visit(jsdoc::VoidLiteral* node);
  inline void Visit(jsdoc::UnionType* node);
  inline void Visit(jsdoc::ArrayType* node);
  inline void Visit(jsdoc::RecordType* node);
  inline void Visit(jsdoc::FieldType* node);
  inline void Visit(jsdoc::FunctionType* node);
  inline void Visit(jsdoc::NameExpression* node);
  inline void Visit(jsdoc::TypeNameWithApplication* node);
  inline void Visit(jsdoc::ParametersType* node);
  inline void Visit(jsdoc::RestExpression* node);
  inline void Visit(jsdoc::PostfixEqualExpression* node);

  Heap* heap_;
  AVal result_;
  std::unordered_set<
      iv::core::UStringPiece,
      UStringPieceHash,
      UStringPieceEquals> lookuped_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_TYPE_INTERPRETER_FWD_H_
