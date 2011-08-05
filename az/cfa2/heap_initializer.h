#ifndef _AZ_CFA2_HEAP_INITIALIZER_H_
#define _AZ_CFA2_HEAP_INITIALIZER_H_
#include <az/cfa2/heap_initializer_fwd.h>
#include <az/cfa2/builtins.h>
namespace az {
namespace cfa2 {

void HeapInitializer::Initialize(FunctionLiteral* global) {
}

// Statements

void HeapInitializer::Visit(Block* block) {
}

void HeapInitializer::Visit(FunctionStatement* func) {
}

void HeapInitializer::Visit(FunctionDeclaration* func) {
}

void HeapInitializer::Visit(VariableStatement* var) {
}

void HeapInitializer::Visit(EmptyStatement* stmt) {
}

void HeapInitializer::Visit(IfStatement* stmt) {
}

void HeapInitializer::Visit(DoWhileStatement* stmt) {
}

void HeapInitializer::Visit(WhileStatement* stmt) {
}

void HeapInitializer::Visit(ForStatement* stmt) {
}

void HeapInitializer::Visit(ForInStatement* stmt) {
}

void HeapInitializer::Visit(ContinueStatement* stmt) {
}

void HeapInitializer::Visit(BreakStatement* stmt) {
}

void HeapInitializer::Visit(ReturnStatement* stmt) {
}

void HeapInitializer::Visit(WithStatement* stmt) {
}

void HeapInitializer::Visit(LabelledStatement* stmt) {
}

void HeapInitializer::Visit(SwitchStatement* stmt) {
}

void HeapInitializer::Visit(ThrowStatement* stmt) {
}

void HeapInitializer::Visit(TryStatement* stmt) {
}

void HeapInitializer::Visit(DebuggerStatement* stmt) {
}

void HeapInitializer::Visit(ExpressionStatement* stmt) {
}

// Expressions

void HeapInitializer::Visit(Assignment* assign) {
}

void HeapInitializer::Visit(BinaryOperation* binary) {
}

void HeapInitializer::Visit(ConditionalExpression* cond) {
}

void HeapInitializer::Visit(UnaryOperation* unary) {
}

void HeapInitializer::Visit(PostfixExpression* postfix) {
}

void HeapInitializer::Visit(StringLiteral* literal) {
}

void HeapInitializer::Visit(NumberLiteral* literal) {
}

void HeapInitializer::Visit(Identifier* literal) {
}

void HeapInitializer::Visit(ThisLiteral* literal) {
  // nothing
}

void HeapInitializer::Visit(NullLiteral* lit) {
  // nothing
}

void HeapInitializer::Visit(TrueLiteral* lit) {
  // nothing
}

void HeapInitializer::Visit(FalseLiteral* lit) {
  // nothing
}

void HeapInitializer::Visit(RegExpLiteral* literal) {
}

void HeapInitializer::Visit(ArrayLiteral* literal) {
}

void HeapInitializer::Visit(ObjectLiteral* literal) {
}

void HeapInitializer::Visit(FunctionLiteral* literal) {
}

void HeapInitializer::Visit(IdentifierAccess* prop) {
}

void HeapInitializer::Visit(IndexAccess* prop) {
}

void HeapInitializer::Visit(FunctionCall* call) {
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void HeapInitializer::Visit(ConstructorCall* call) {
  heap_->DeclObject(call, heap_->GetFactory()->NewAObject());
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

// Others

void HeapInitializer::Visit(Declaration* dummy) {
}

void HeapInitializer::Visit(CaseClause* clause) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_DECL_INITIALIZER_H_
