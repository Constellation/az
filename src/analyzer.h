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

  class FlowSwitcher : private iv::core::Noncopyable<FlowSwitcher> {
   public:
    FlowSwitcher(Analyzer* analyzer)
      : analyzer_(analyzer) {
    }
   private:
    Analyzer* analyzer_;
    Statement* normal_;
    Statement* raised_;
  };

  void Analyze(FunctionLiteral* global) {
    normal_ = NULL;
    AnalyzeStatements(global->body());
  }

 private:
  void AnalyzeStatements(const Statements& stmts) {
    Statement* normal = normal_;
    for (Statements::const_iterator it = stmts.begin(),
         start = stmts.begin(), last = stmts.end(); it != last; ++it) {
      Statements::const_iterator next = it;
      ++next;
      if (next != last) {
        normal_ = *next;
      } else {
        normal_ = normal;
      }
      (*it)->Accept(this);
    }
  }

  void Visit(Block* block) {
    block->set_normal(normal_);
    AnalyzeStatements(block->body());
  }

  void Visit(FunctionStatement* func) {
    func->set_normal(normal_);
  }

  void Visit(FunctionDeclaration* func) {
    func->set_normal(normal_);
  }

  void Visit(VariableStatement* var) {
    var->set_normal(normal_);
  }

  void Visit(EmptyStatement* stmt) {
    stmt->set_normal(normal_);
  }

  void Visit(IfStatement* stmt) {
    // TODO(Constellation) analyze then and else
    stmt->set_normal(normal_);
  }

  void Visit(DoWhileStatement* stmt) {
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(WhileStatement* stmt) {
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(ForStatement* stmt) {
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(ForInStatement* stmt) {
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(ContinueStatement* stmt) {
    // TODO(Constellation) analyze continue jump
    stmt->set_normal(stmt->target());
  }

  void Visit(BreakStatement* stmt) {
    // TODO(Constellation) analyze break jump
    stmt->set_normal(stmt->target());
  }

  void Visit(ReturnStatement* stmt) {
    // TODO(Constellation) analyze return
    stmt->set_normal(normal_);
  }

  void Visit(WithStatement* stmt) {
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(LabelledStatement* stmt) {
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(SwitchStatement* stmt) {
    // TODO(Constellation) analyze switch
    stmt->set_normal(normal_);
  }

  void Visit(ThrowStatement* stmt) {
    // TODO(Constellation) analyze throw
    stmt->set_normal(normal_);
  }

  void Visit(TryStatement* stmt) {
    // TODO(Constellation) analyze try
    stmt->set_normal(normal_);
  }

  void Visit(DebuggerStatement* stmt) {
    stmt->set_normal(normal_);
  }

  void Visit(ExpressionStatement* stmt) {
    stmt->set_normal(normal_);
  }

  void Visit(Assignment* assign) {
  }

  void Visit(BinaryOperation* binary) {
  }

  void Visit(ConditionalExpression* cond) {
  }

  void Visit(UnaryOperation* unary) {
  }

  void Visit(PostfixExpression* postfix) {
  }

  void Visit(StringLiteral* literal) {
  }

  void Visit(NumberLiteral* literal) {
  }

  void Visit(Identifier* literal) {
  }

  void Visit(ThisLiteral* literal) {
  }

  void Visit(NullLiteral* lit) {
  }

  void Visit(TrueLiteral* lit) {
  }

  void Visit(FalseLiteral* lit) {
  }

  void Visit(RegExpLiteral* literal) {
  }

  void Visit(ArrayLiteral* literal) {
  }

  void Visit(ObjectLiteral* literal) {
  }

  void Visit(FunctionLiteral* literal) {
  }

  void Visit(IdentifierAccess* prop) {
  }

  void Visit(IndexAccess* prop) {
  }

  void Visit(FunctionCall* call) {
  }

  void Visit(ConstructorCall* call) {
  }

  void Visit(Declaration* dummy) {
  }

  void Visit(CaseClause* dummy) {
  }

  Statement* normal_;
  Statement* raised_;
};

}  // namespace az
#endif  // _AZ_ANALYZER_H_
