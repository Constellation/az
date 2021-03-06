#ifndef AZ_CFA2_JSDOC_COLLECTOR_FWD_H_
#define AZ_CFA2_JSDOC_COLLECTOR_FWD_H_
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <az/symbol.h>
namespace az {
namespace cfa2 {

class JSDocCollector
  : private iv::core::Noncopyable<JSDocCollector>,
    public az::MutableAstVisitor {
 public:
  explicit JSDocCollector(Heap* heap)
    : heap_(heap) {
  }

  inline void Collect(FunctionLiteral* global);

 private:
  inline void Visit(Block* block);
  inline void Visit(FunctionStatement* func);
  inline void Visit(FunctionDeclaration* func);
  inline void Visit(VariableStatement* var);
  inline void Visit(EmptyStatement* stmt);
  inline void Visit(IfStatement* stmt);
  inline void Visit(DoWhileStatement* stmt);
  inline void Visit(WhileStatement* stmt);
  inline void Visit(ForStatement* stmt);
  inline void Visit(ForInStatement* stmt);
  inline void Visit(ContinueStatement* stmt);
  inline void Visit(BreakStatement* stmt);
  inline void Visit(ReturnStatement* stmt);
  inline void Visit(WithStatement* stmt);
  inline void Visit(LabelledStatement* stmt);
  inline void Visit(SwitchStatement* stmt);
  inline void Visit(ThrowStatement* stmt);
  inline void Visit(TryStatement* stmt);
  inline void Visit(DebuggerStatement* stmt);
  inline void Visit(ExpressionStatement* stmt);

  inline void Visit(Assignment* assign);
  inline void Visit(BinaryOperation* binary);
  inline void Visit(ConditionalExpression* cond);
  inline void Visit(UnaryOperation* unary);
  inline void Visit(PostfixExpression* postfix);
  inline void Visit(StringLiteral* literal);
  inline void Visit(NumberLiteral* literal);
  inline void Visit(Identifier* literal);
  inline void Visit(Assigned* ident);
  inline void Visit(ThisLiteral* literal);
  inline void Visit(NullLiteral* lit);
  inline void Visit(TrueLiteral* lit);
  inline void Visit(FalseLiteral* lit);
  inline void Visit(RegExpLiteral* literal);
  inline void Visit(ArrayLiteral* literal);
  inline void Visit(ObjectLiteral* literal);
  inline void Visit(FunctionLiteral* literal);
  inline void Visit(IdentifierAccess* prop);
  inline void Visit(IndexAccess* prop);
  inline void Visit(FunctionCall* call);
  inline void Visit(ConstructorCall* call);

  inline void Visit(Declaration* dummy);
  inline void Visit(CaseClause* clause);

  Heap* heap_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_JSDOC_COLLECTOR_FWD_H_
