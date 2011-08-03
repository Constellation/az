// CFA2 interpreter
#ifndef _AZ_CFA2_INTERPRETER_FWD_H_
#define _AZ_CFA2_INTERPRETER_FWD_H_
#include <vector>
#include <iv/detail/cstdint.h>
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <az/jstype.h>
#include <az/cfa2/heap.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/completer.h>
namespace az {
namespace cfa2 {

class Interpreter
  : private iv::core::Noncopyable<Interpreter>,
    public MutableAstVisitor {
 public:
  typedef std::vector<Binding*> Bindings;

  explicit Interpreter(Heap* heap,
                       Completer* completer)
    : heap_(heap),
      completer_(completer) {
  }

  inline void Run(FunctionLiteral* global);

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
  Completer* completer_;
  uint64_t timestamp_;
};

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_INTERPRETER_FWD_H_
