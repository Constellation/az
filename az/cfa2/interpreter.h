#ifndef _AZ_CFA2_INTERPRETER_H_
#define _AZ_CFA2_INTERPRETER_H_
#include <iv/debug.h>
#include <az/cfa2/interpreter_fwd.h>
#include <az/cfa2/frame.h>
namespace az {
namespace cfa2 {

class Work { };

void Interpreter::Run(FunctionLiteral* global) {
  Frame frame;
  std::cout << "INTERPRETER START" << std::endl;

  // decl initializer

  frame.SetThis(heap_->GetGlobal());
  const Scope& scope = global->scope();
  for (Scope::Variables::const_iterator it = scope.variables().begin(),
       last = scope.variables().end(); it != last; ++it) {
    const Scope::Variable& var = *it;
    Identifier* ident = var.first;
    assert(ident->refer());
    frame.Set(heap_, ident->refer(), AVal(AVAL_NOBASE));
  }

  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    const Symbol fn = Intern((*it)->name().Address()->value());
    Identifier* ident = (*it)->name().Address();
    assert(ident->refer());
    frame.Set(heap_, ident->refer(), AVal(AVAL_NOBASE));
  }

  // interpret global function
  for (Statements::const_iterator it = global->body().begin(),
       last = global->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }

  // summary update phase
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
