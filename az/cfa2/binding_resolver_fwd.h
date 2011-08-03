// resolve which identifier is STACK or HEAP
#ifndef _AZ_CFA2_BINDING_RESOLVER_FWD_H_
#define _AZ_CFA2_BINDING_RESOLVER_FWD_H_
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <az/symbol.h>
#include <az/cfa2/binding.h>
namespace az {
namespace cfa2 {

class BindingResolver
  : private iv::core::Noncopyable<BindingResolver>,
    public az::MutableAstVisitor {
 public:
  typedef std::vector<Binding*> Bindings;


  explicit BindingResolver(Heap* heap)
    : heap_(heap), inner_scope_(NULL), outer_scope_() { }

  inline void Resolve(FunctionLiteral* global);

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
  // CFA2 Variable Stacks
  Bindings* inner_scope_;
  Bindings outer_scope_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BINDING_RESOLVER_FWD_H_
