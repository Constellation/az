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
namespace detail {

const Statement* kNextStatement = NULL;

}  // namespace detail

template<typename Reporter>
class Analyzer
  : public iv::core::ast::AstVisitor<AstFactory>::type,
    private iv::core::Noncopyable<Analyzer<Reporter> > {
 public:

  Analyzer(Reporter* reporter)
    : normal_(NULL),
      raised_(NULL),
      map_(),
      current_function_info_(),
      reporter_(reporter),
      current_continuation_set_(NULL) {
  }

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
    CheckDeadStatement(block);
    StoreStatement(block);
    block->set_normal(normal_);
    AnalyzeStatements(block->body());

    ResolveContinuationJump(block);
  }

  void Visit(FunctionStatement* func) {
    CheckDeadStatement(func);
    StoreStatement(func);
    func->set_normal(normal_);

    // analyze function
    Visit(func->function());
  }

  void Visit(FunctionDeclaration* func) {
    CheckDeadStatement(func);
    StoreStatement(func);
    func->set_normal(normal_);

    // analyze function
    Visit(func->function());
  }

  void Visit(VariableStatement* var) {
    CheckDeadStatement(var);
    StoreStatement(var);
    var->set_normal(normal_);
  }

  void Visit(EmptyStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(IfStatement* stmt) {
    // TODO(Constellation) analyze then and else
    CheckDeadStatement(stmt);

    // IfStatement
    // this statement have branch

    if (stmt->else_statement()) {
      // else statement exists
      Statement* else_statement = stmt->else_statement().Address();
      current_continuation_set_->insert(else_statement);
      stmt->then_statement()->Accept(this);
      current_continuation_set_->erase(else_statement);
      if (IsDeadStatement()) {
        current_continuation_set_->insert(detail::kNextStatement);
      } else {
        current_continuation_set_->insert(stmt);
      }
      else_statement->Accept(this);
      if (current_continuation_set_->count(stmt) != 0) {
        current_continuation_set_->erase(stmt);
        if (IsDeadStatement()) {
          current_continuation_set_->insert(detail::kNextStatement);
        }
      }
    } else {
      // then statement only
      stmt->then_statement()->Accept(this);
      if (IsDeadStatement()) {
        // recover
        current_continuation_set_->insert(detail::kNextStatement);
      }
    }

    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(DoWhileStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
    ResolveContinuationJump(stmt);
  }

  void Visit(WhileStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
    ResolveContinuationJump(stmt);
  }

  void Visit(ForStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
    ResolveContinuationJump(stmt);
  }

  void Visit(ForInStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
    ResolveContinuationJump(stmt);
  }

  void Visit(ContinueStatement* stmt) {
    // TODO(Constellation) analyze continue jump
    CheckDeadStatement(stmt);

    // ContinueStatement
    // remove kNextStatement and add Target
    current_continuation_set_->erase(detail::kNextStatement);
    current_continuation_set_->insert(stmt->target());

    StoreStatement(stmt);
    stmt->set_normal(stmt->target());
  }

  void Visit(BreakStatement* stmt) {
    // TODO(Constellation) analyze break jump
    CheckDeadStatement(stmt);

    // BreakStatement
    // remove kNextStatement and add Target
    current_continuation_set_->erase(detail::kNextStatement);
    current_continuation_set_->insert(stmt->target());

    StoreStatement(stmt);
    stmt->set_normal(stmt->target());
  }

  void Visit(ReturnStatement* stmt) {
    // TODO(Constellation) analyze return
    CheckDeadStatement(stmt);

    // ReturnStatement
    // remove kNextStatement
    current_continuation_set_->erase(detail::kNextStatement);

    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(WithStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(LabelledStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(SwitchStatement* stmt) {
    CheckDeadStatement(stmt);
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
    ResolveContinuationJump(stmt);
  }

  void Visit(ThrowStatement* stmt) {
    // TODO(Constellation) analyze throw
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(TryStatement* stmt) {
    // TODO(Constellation) analyze try
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(DebuggerStatement* stmt) {
    CheckDeadStatement(stmt);
    StoreStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(ExpressionStatement* stmt) {
    CheckDeadStatement(stmt);
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
    // TODO(Constellation)
    // create RAII object
    std::shared_ptr<FunctionInfo> current_info(new FunctionInfo());
    (*map_)[literal] = current_info;
    std::unordered_set<const Statement*> current_continuation_set;
    std::unordered_set<const Statement*>* prev = current_continuation_set_;
    current_continuation_set_ = &current_continuation_set;
    current_continuation_set_->insert(detail::kNextStatement);  // next statement continuation is default
    {
      FunctionInfoSwitcher switcher(this, current_info);

      const Scope& scope = literal->scope();
      {
        // function declarations
        typedef Scope::FunctionLiterals Functions;
        const Functions& functions = scope.function_declarations();
        for (Functions::const_iterator it = functions.begin(),
             last = functions.end(); it != last; ++it) {
          StoreVariable((*it)->name().Address());
        }
      }
      {
        // variables
        typedef Scope::Variables Variables;
        const Variables& vars = scope.variables();
        for (Variables::const_iterator it = vars.begin(),
             last = vars.end(); it != last; ++it) {
          StoreVariable(it->first);
        }
      }
      AnalyzeStatements(literal->body());
    }
    current_continuation_set_ = prev;
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
    // iv::core::unicode::FPutsUTF16(stdout, key.begin(), key.end());
    current_function_info_->first.insert(std::make_pair(key, std::make_pair(type, TypeSet())));
  }

  void CheckDeadStatement(const Statement* stmt) {
    if (IsDeadStatement()) {
      reporter_->ReportDeadStatement(*stmt);
    }
  }

  bool IsDeadStatement() const {
    return current_continuation_set_->count(detail::kNextStatement) == 0;
  }

  void ResolveContinuationJump(const BreakableStatement* target) {
    if (current_continuation_set_->count(target) != 0) {
      current_continuation_set_->erase(target);
      current_continuation_set_->insert(detail::kNextStatement);
    }
  }

  Statement* normal_;
  Statement* raised_;
  std::shared_ptr<FunctionMap> map_;
  std::shared_ptr<FunctionInfo> current_function_info_;
  Reporter* reporter_;
  std::unordered_set<const Statement*>* current_continuation_set_;
};

template<typename Source, typename Reporter>
inline void Analyze(FunctionLiteral* global, const Source& src, Reporter* reporter) {
  Analyzer<Reporter> analyzer(reporter);
  std::shared_ptr<FunctionMap> result = analyzer.Analyze(global);
}

}  // namespace az
#endif  // _AZ_ANALYZER_H_
