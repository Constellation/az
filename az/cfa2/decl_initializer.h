#ifndef _AZ_CFA2_DECL_INITIALIZER_H_
#define _AZ_CFA2_DECL_INITIALIZER_H_
#include <az/cfa2/decl_initializer_fwd.h>
#include <az/cfa2/builtins.h>
namespace az {
namespace cfa2 {

void DeclInitializer::Initialize(FunctionLiteral* global) {
}

// Statements

void DeclInitializer::Visit(Block* block) {
}

void DeclInitializer::Visit(FunctionStatement* func) {
}

void DeclInitializer::Visit(FunctionDeclaration* func) {
}

void DeclInitializer::Visit(VariableStatement* var) {
}

void DeclInitializer::Visit(EmptyStatement* stmt) {
}

void DeclInitializer::Visit(IfStatement* stmt) {
}

void DeclInitializer::Visit(DoWhileStatement* stmt) {
}

void DeclInitializer::Visit(WhileStatement* stmt) {
}

void DeclInitializer::Visit(ForStatement* stmt) {
}

void DeclInitializer::Visit(ForInStatement* stmt) {
}

void DeclInitializer::Visit(ContinueStatement* stmt) {
}

void DeclInitializer::Visit(BreakStatement* stmt) {
}

void DeclInitializer::Visit(ReturnStatement* stmt) {
}

void DeclInitializer::Visit(WithStatement* stmt) {
}

void DeclInitializer::Visit(LabelledStatement* stmt) {
}

void DeclInitializer::Visit(SwitchStatement* stmt) {
}

void DeclInitializer::Visit(ThrowStatement* stmt) {
}

void DeclInitializer::Visit(TryStatement* stmt) {
}

void DeclInitializer::Visit(DebuggerStatement* stmt) {
}

void DeclInitializer::Visit(ExpressionStatement* stmt) {
}

// Expressions

void DeclInitializer::Visit(Assignment* assign) {
}

void DeclInitializer::Visit(BinaryOperation* binary) {
}

void DeclInitializer::Visit(ConditionalExpression* cond) {
}

void DeclInitializer::Visit(UnaryOperation* unary) {
}

void DeclInitializer::Visit(PostfixExpression* postfix) {
}

void DeclInitializer::Visit(StringLiteral* literal) {
}

void DeclInitializer::Visit(NumberLiteral* literal) {
}

void DeclInitializer::Visit(Identifier* literal) {
}

void DeclInitializer::Visit(ThisLiteral* literal) {
  // nothing
}

void DeclInitializer::Visit(NullLiteral* lit) {
  // nothing
}

void DeclInitializer::Visit(TrueLiteral* lit) {
  // nothing
}

void DeclInitializer::Visit(FalseLiteral* lit) {
  // nothing
}

void DeclInitializer::Visit(RegExpLiteral* literal) {
}

void DeclInitializer::Visit(ArrayLiteral* literal) {
}

void DeclInitializer::Visit(ObjectLiteral* literal) {
}

void DeclInitializer::Visit(FunctionLiteral* literal) {
}

void DeclInitializer::Visit(IdentifierAccess* prop) {
}

void DeclInitializer::Visit(IndexAccess* prop) {
}

void DeclInitializer::Visit(FunctionCall* call) {
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void DeclInitializer::Visit(ConstructorCall* call) {
  heap_->DeclObject(call, heap_->GetFactory()->NewAObject());
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

// Others

void DeclInitializer::Visit(Declaration* dummy) {
}

void DeclInitializer::Visit(CaseClause* clause) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_DECL_INITIALIZER_H_
