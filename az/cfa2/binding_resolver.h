// resolve which identifier is STACK or HEAP
#ifndef _AZ_CFA2_BINDING_RESOLVER_H_
#define _AZ_CFA2_BINDING_RESOLVER_H_
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <az/symbol.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/binding_resolver_fwd.h>
namespace az {
namespace cfa2 {

void BindingResolver::Visit(Block* block) {
}

void BindingResolver::Visit(FunctionStatement* func) {
}

void BindingResolver::Visit(FunctionDeclaration* func) {
}

void BindingResolver::Visit(VariableStatement* var) {
}

void BindingResolver::Visit(EmptyStatement* stmt) {
}

void BindingResolver::Visit(IfStatement* stmt) {
}

void BindingResolver::Visit(DoWhileStatement* stmt) {
}

void BindingResolver::Visit(WhileStatement* stmt) {
}

void BindingResolver::Visit(ForStatement* stmt) {
}

void BindingResolver::Visit(ForInStatement* stmt) {
}

void BindingResolver::Visit(ContinueStatement* stmt) {
}

void BindingResolver::Visit(BreakStatement* stmt) {
}

void BindingResolver::Visit(ReturnStatement* stmt) {
}

void BindingResolver::Visit(WithStatement* stmt) {
}

void BindingResolver::Visit(LabelledStatement* stmt) {
}

void BindingResolver::Visit(SwitchStatement* stmt) {
}

void BindingResolver::Visit(ThrowStatement* stmt) {
}

void BindingResolver::Visit(TryStatement* stmt) {
}

void BindingResolver::Visit(DebuggerStatement* stmt) {
}

void BindingResolver::Visit(ExpressionStatement* stmt) {
}


void BindingResolver::Visit(Assignment* assign) {
}

void BindingResolver::Visit(BinaryOperation* binary) {
}

void BindingResolver::Visit(ConditionalExpression* cond) {
}

void BindingResolver::Visit(UnaryOperation* unary) {
}

void BindingResolver::Visit(PostfixExpression* postfix) {
}

void BindingResolver::Visit(StringLiteral* literal) {
}

void BindingResolver::Visit(NumberLiteral* literal) {
}

void BindingResolver::Visit(Identifier* literal) {
}

void BindingResolver::Visit(ThisLiteral* literal) {
}

void BindingResolver::Visit(NullLiteral* lit) {
}

void BindingResolver::Visit(TrueLiteral* lit) {
}

void BindingResolver::Visit(FalseLiteral* lit) {
}

void BindingResolver::Visit(RegExpLiteral* literal) {
}

void BindingResolver::Visit(ArrayLiteral* literal) {
}

void BindingResolver::Visit(ObjectLiteral* literal) {
}

void BindingResolver::Visit(FunctionLiteral* literal) {
}

void BindingResolver::Visit(IdentifierAccess* prop) {
}

void BindingResolver::Visit(IndexAccess* prop) {
}

void BindingResolver::Visit(FunctionCall* call) {
}

void BindingResolver::Visit(ConstructorCall* call) {
}


void BindingResolver::Visit(Declaration* dummy) {
}

void BindingResolver::Visit(CaseClause* dummy) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BINDING_RESOLVER_H_
