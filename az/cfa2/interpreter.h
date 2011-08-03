#ifndef _AZ_CFA2_INTERPRETER_H_
#define _AZ_CFA2_INTERPRETER_H_
#include <az/cfa2/interpreter_fwd.h>
namespace az {
namespace cfa2 {

void Interpreter::Run(FunctionLiteral* global) {
}

// Statements

void Interpreter::Visit(Block* block) {
}

void Interpreter::Visit(FunctionStatement* func) {
}

void Interpreter::Visit(FunctionDeclaration* func) {
}

void Interpreter::Visit(VariableStatement* var) {
}

void Interpreter::Visit(EmptyStatement* stmt) {
}

void Interpreter::Visit(IfStatement* stmt) {
}

void Interpreter::Visit(DoWhileStatement* stmt) {
}

void Interpreter::Visit(WhileStatement* stmt) {
}

void Interpreter::Visit(ForStatement* stmt) {
}

void Interpreter::Visit(ForInStatement* stmt) {
}

void Interpreter::Visit(ContinueStatement* stmt) {
}

void Interpreter::Visit(BreakStatement* stmt) {
}

void Interpreter::Visit(ReturnStatement* stmt) {
}

void Interpreter::Visit(WithStatement* stmt) {
}

void Interpreter::Visit(LabelledStatement* stmt) {
}

void Interpreter::Visit(SwitchStatement* stmt) {
}

void Interpreter::Visit(ThrowStatement* stmt) {
}

void Interpreter::Visit(TryStatement* stmt) {
}

void Interpreter::Visit(DebuggerStatement* stmt) {
}

void Interpreter::Visit(ExpressionStatement* stmt) {
}

// Expressions

void Interpreter::Visit(Assignment* assign) {
}

void Interpreter::Visit(BinaryOperation* binary) {
}

void Interpreter::Visit(ConditionalExpression* cond) {
}

void Interpreter::Visit(UnaryOperation* unary) {
}

void Interpreter::Visit(PostfixExpression* postfix) {
}

void Interpreter::Visit(StringLiteral* literal) {
}

void Interpreter::Visit(NumberLiteral* literal) {
}

void Interpreter::Visit(Identifier* literal) {
}

void Interpreter::Visit(ThisLiteral* literal) {
}

void Interpreter::Visit(NullLiteral* lit) {
}

void Interpreter::Visit(TrueLiteral* lit) {
}

void Interpreter::Visit(FalseLiteral* lit) {
}

void Interpreter::Visit(RegExpLiteral* literal) {
}

void Interpreter::Visit(ArrayLiteral* literal) {
}

void Interpreter::Visit(ObjectLiteral* literal) {
}

void Interpreter::Visit(FunctionLiteral* literal) {
}

void Interpreter::Visit(IdentifierAccess* prop) {
}

void Interpreter::Visit(IndexAccess* prop) {
}

void Interpreter::Visit(FunctionCall* call) {
}

void Interpreter::Visit(ConstructorCall* call) {
}

// Others

void Interpreter::Visit(Declaration* dummy) {
}

void Interpreter::Visit(CaseClause* clause) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_INTERPRETER_H_
