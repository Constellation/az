// Az Analyzer
// rewrite AST and analyze stack and heap variables status
#ifndef _AZ_ANALYZER_H_
#define _AZ_ANALYZER_H_
#include <iv/ast_visitor.h>
#include <iv/utils.h>
#include <iv/maybe.h>
#include <iv/noncopyable.h>
#include <iv/unicode.h>
#include "ast_fwd.h"
#include "factory.h"
#include "analyze_map.h"

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

  std::shared_ptr<FunctionMap> Analyze(FunctionLiteral* global) {
    // Global Settings
    normal_ = NULL;
    std::shared_ptr<FunctionMap> result(new FunctionMap());
    map_ = result;

    Visit(global);

    map_.reset();
    return result;
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
    StoreStatement(block);
    block->set_normal(normal_);
    AnalyzeStatements(block->body());
  }

  void Visit(FunctionStatement* func) {
    StoreStatement(func);
    func->set_normal(normal_);

    // analyze function
    Visit(func->function());
  }

  void Visit(FunctionDeclaration* func) {
    StoreStatement(func);
    func->set_normal(normal_);

    // analyze function
    Visit(func->function());
  }

  void Visit(VariableStatement* var) {
    StoreStatement(var);
    var->set_normal(normal_);
  }

  void Visit(EmptyStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(IfStatement* stmt) {
    // TODO(Constellation) analyze then and else
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(DoWhileStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(WhileStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(ForStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(ForInStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(ContinueStatement* stmt) {
    // TODO(Constellation) analyze continue jump
    StoreStatement(stmt);
    stmt->set_normal(stmt->target());
  }

  void Visit(BreakStatement* stmt) {
    // TODO(Constellation) analyze break jump
    StoreStatement(stmt);
    stmt->set_normal(stmt->target());
  }

  void Visit(ReturnStatement* stmt) {
    // TODO(Constellation) analyze return
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(WithStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(LabelledStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(SwitchStatement* stmt) {
    StoreStatement(stmt);
    typedef SwitchStatement::CaseClauses CaseClauses;
    const CaseClauses& clauses = stmt->clauses();
    Statement* normal = normal_;
    for (CaseClauses::const_iterator it = clauses.begin(),
         start = clauses.begin(), last = clauses.end(); it != last; ++it) {
      CaseClauses::const_iterator next = it;
      ++next;
      if (next != last) {
        // search next stmt
        for (;next != last; ++next) {
          if (!(*next)->body().empty()) {
            // found next stmt
            normal_ = (*next)->body().front();
            break;
          }
        }
        if (next == last) {
          normal_ = normal;
        }
      } else {
        normal_ = normal;
      }
      AnalyzeStatements((*it)->body());
    }
  }

  void Visit(ThrowStatement* stmt) {
    // TODO(Constellation) analyze throw
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(TryStatement* stmt) {
    // TODO(Constellation) analyze try
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(DebuggerStatement* stmt) {
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(ExpressionStatement* stmt) {
    StoreStatement(stmt);
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

  class FunctionInfoSwitcher : private iv::core::Noncopyable<FunctionInfoSwitcher> {
   public:
    FunctionInfoSwitcher(Analyzer* analyzer, std::shared_ptr<FunctionInfo> info)
      : analyzer_(analyzer),
        prev_(analyzer->current_function_info()) {
      analyzer_->set_current_function_info(info);
    }

    ~FunctionInfoSwitcher() {
      analyzer_->set_current_function_info(prev_);
    }

   private:
    Analyzer* analyzer_;
    std::shared_ptr<FunctionInfo> prev_;
  };

  void Visit(FunctionLiteral* literal) {
    std::shared_ptr<FunctionInfo> current_info(new FunctionInfo());
    (*map_)[literal] = current_info;
    FunctionInfoSwitcher switcher(this, current_info);
    AnalyzeStatements(literal->body());
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

  std::shared_ptr<FunctionInfo> current_function_info() const {
    return current_function_info_;
  }

  void set_current_function_info(std::shared_ptr<FunctionInfo> info) {
    current_function_info_ = info;
  }

  // remember this statement is located at this function
  void StoreStatement(Statement* stmt) {
    current_function_info_->second.insert(std::make_pair(stmt, ReachableAndResult()));
  }

  // remember this variable is located at this function stack
  void StoreVariable(Identifier* ident,
                     VariableType type = VARIABLE_STACK) {
    const iv::core::UString key(ident->value().begin(), ident->value().end());
    iv::core::unicode::FPutsUTF16(stdout, key.begin(), key.end());
    current_function_info_->first.insert(std::make_pair(key, std::make_pair(type, TypeSet())));
  }

  Statement* normal_;
  Statement* raised_;
  std::shared_ptr<FunctionMap> map_;
  std::shared_ptr<FunctionInfo> current_function_info_;
};

template<typename Source>
inline void Analyze(FunctionLiteral* global, const Source& src) {
  Analyzer analyzer;
  std::shared_ptr<FunctionMap> result = analyzer.Analyze(global);
}

}  // namespace az
#endif  // _AZ_ANALYZER_H_
