// Az Analyzer
// rewrite AST and analyze stack and heap variables status
#ifndef _AZ_ANALYZER_H_
#define _AZ_ANALYZER_H_
#include <iv/ast_visitor.h>
#include <iv/utils.h>
#include <iv/maybe.h>
#include <iv/noncopyable.h>
#include "ast_fwd.h"
#include "factory.h"

namespace az {

class Analyzer
  : public iv::core::ast::AstVisitor<AstFactory>::type,
    private iv::core::Noncopyable<Analyzer> {
 public:
  void Analyze(FunctionLiteral* literal) { }

  void Visit(Block* block) { }
  void Visit(FunctionStatement* func) { }
  void Visit(FunctionDeclaration* func) { }
  void Visit(VariableStatement* var) { }
  void Visit(EmptyStatement* stmt) { }
  void Visit(IfStatement* stmt) { }
  void Visit(DoWhileStatement* stmt) { }
  void Visit(WhileStatement* stmt) { }
  void Visit(ForStatement* stmt) { }
  void Visit(ForInStatement* stmt) { }
  void Visit(ContinueStatement* stmt) { }
  void Visit(BreakStatement* stmt) { }
  void Visit(ReturnStatement* stmt) { }
  void Visit(WithStatement* stmt) { }
  void Visit(LabelledStatement* stmt) { }
  void Visit(SwitchStatement* stmt) { }
  void Visit(ThrowStatement* stmt) { }
  void Visit(TryStatement* stmt) { }
  void Visit(DebuggerStatement* stmt) { }
  void Visit(ExpressionStatement* stmt) { }
  void Visit(Assignment* assign) { }
  void Visit(BinaryOperation* binary) { }
  void Visit(ConditionalExpression* cond) { }
  void Visit(UnaryOperation* unary) { }
  void Visit(PostfixExpression* postfix) { }
  void Visit(StringLiteral* literal) { }
  void Visit(NumberLiteral* literal) { }
  void Visit(Identifier* literal) { }
  void Visit(ThisLiteral* literal) { }
  void Visit(NullLiteral* lit) { }
  void Visit(TrueLiteral* lit) { }
  void Visit(FalseLiteral* lit) { }
  void Visit(RegExpLiteral* literal) { }
  void Visit(ArrayLiteral* literal) { }
  void Visit(ObjectLiteral* literal) { }
  void Visit(FunctionLiteral* literal) { }
  void Visit(IdentifierAccess* prop) { }
  void Visit(IndexAccess* prop) { }
  void Visit(FunctionCall* call) { }
  void Visit(ConstructorCall* call) { }
  void Visit(Declaration* dummy) { }
  void Visit(CaseClause* dummy) { }
};

}  // namespace az
#endif  // _AZ_ANALYZER_H_
