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
#include "environment.h"
#include "execution_context.h"

namespace az {
namespace detail {

const Statement* kNextStatement = NULL;

}  // namespace detail

class StatusSetRAII;

class ContinuationStatus : private iv::core::Noncopyable<ContinuationStatus> {
 public:
  friend class StatusSetRAII;
  typedef std::unordered_set<const Statement*> ContinuationSet;

  ContinuationStatus()
    : current_(NULL) {
  }

  void Insert(const Statement* stmt) {
    current_->insert(stmt);
  }

  void Erase(const Statement* stmt) {
    current_->erase(stmt);
  }

  void Kill() {
    Erase(detail::kNextStatement);
  }

  bool Has(const Statement* stmt) const {
    return current_->count(stmt) != 0;
  }

  bool IsDeadStatement() const {
    return !Has(detail::kNextStatement);
  }

  void JumpTo(const BreakableStatement* target) {
    Kill();
    Insert(target);
  }

  void ResolveJump(const BreakableStatement* target) {
    if (Has(target)) {
      Erase(target);
      Insert(detail::kNextStatement);
    }
  }

 private:

  void set_current(ContinuationSet* set) {
    current_ = set;
  }

  ContinuationSet* current() const {
    return current_;
  }

  ContinuationSet* current_;
};

class StatusSetRAII : private iv::core::Noncopyable<StatusSetRAII> {
 public:
  StatusSetRAII(ContinuationStatus* status)
    : status_(status),
      previous_(status->current()),
      set_() {
    set_.insert(detail::kNextStatement);
    status->set_current(&set_);
  }

  ~StatusSetRAII() {
    status_->set_current(previous_);
  }

 private:
  ContinuationStatus* status_;
  ContinuationStatus::ContinuationSet* previous_;
  ContinuationStatus::ContinuationSet set_;
};

template<typename Reporter>
class Analyzer
  : public iv::core::ast::AstVisitor<AstFactory>::type,
    private iv::core::Noncopyable<Analyzer<Reporter> > {
 public:

  Analyzer(Reporter* reporter)
    : normal_(NULL),
      raised_(NULL),
      reporter_(reporter),
      status_() {
  }

  void Analyze(FunctionLiteral* global) {
    // Global Settings
    normal_ = NULL;
    std::shared_ptr<ExecutionContext> ctx = ExecutionContext::CreateGlobal(global);
    AnalyzeFunctionLiteral(global, ctx);
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

  void AnalyzeFunctionLiteral(FunctionLiteral* literal,
                              std::shared_ptr<ExecutionContext> ctx) {
    const StatusSetRAII status_set_raii(&status_);
    ExecutionSwitcher switcher(this, ctx);

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

  void Visit(Block* block) {
    CheckDeadStatement(block);
    const bool dead = IsDeadStatement();

    block->set_normal(normal_);
    AnalyzeStatements(block->body());

    if (!dead) {
      status_.ResolveJump(block);
    }
  }

  void Visit(FunctionStatement* stmt) {
    CheckDeadStatement(stmt);

    // report FunctionStatement
    reporter_->ReportFunctionStatement(stmt);

    stmt->set_normal(normal_);

    // variable analyze
    FunctionLiteral* literal = stmt->function();

    // the name is trapped, get environment.
    std::shared_ptr<Environment> target = context_->GetLexicalEnvironment()->Lookup(literal->name().Address()->value());

    // analyze function
    Visit(literal);
  }

  void Visit(FunctionDeclaration* func) {
    CheckDeadStatement(func);
    func->set_normal(normal_);

    // analyze function
    Visit(func->function());
  }

  void Visit(VariableStatement* var) {
    CheckDeadStatement(var);
    var->set_normal(normal_);
  }

  void Visit(EmptyStatement* stmt) {
    CheckDeadStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(IfStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    // IfStatement
    // this statement have branch
    if (stmt->else_statement()) {
      // else statement exists
      Statement* else_statement = stmt->else_statement().Address();
      stmt->then_statement()->Accept(this);
      if (!dead) {
        if (IsDeadStatement()) {
          status_.Insert(detail::kNextStatement);
        } else {
          status_.Insert(stmt);
        }
      }
      else_statement->Accept(this);
      if (!dead) {
        if (status_.Has(stmt)) {
          status_.Erase(stmt);
          if (IsDeadStatement()) {
            status_.Insert(detail::kNextStatement);
          }
        }
      }
    } else {
      // then statement only
      stmt->then_statement()->Accept(this);
      if (!dead && IsDeadStatement()) {
        // recover if this IfStatement is not dead code
        status_.Insert(detail::kNextStatement);
      }
    }

    stmt->set_normal(normal_);
  }

  void Visit(DoWhileStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);

    if (!dead) {
      status_.ResolveJump(stmt);
    }
  }

  void Visit(WhileStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);

    if (!dead) {
      status_.ResolveJump(stmt);
    }
  }

  void Visit(ForStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);

    if (!dead) {
      status_.ResolveJump(stmt);
    }
  }

  void Visit(ForInStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);

    if (!dead) {
      status_.ResolveJump(stmt);
    }
  }

  void Visit(ContinueStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    if (!dead) {
      status_.JumpTo(stmt->target());
    }

    stmt->set_normal(stmt->target());
  }

  void Visit(BreakStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    if (!dead) {
      status_.JumpTo(stmt->target());
    }

    stmt->set_normal(stmt->target());
  }

  void Visit(ReturnStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    if (!dead) {
      status_.Kill();
    }

    stmt->set_normal(normal_);
  }

  void Visit(WithStatement* stmt) {
    CheckDeadStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(LabelledStatement* stmt) {
    CheckDeadStatement(stmt);
    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);
  }

  void Visit(SwitchStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

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

      if (!dead) {
        if (IsDeadStatement()) {
          if ((it + 1) != last) {
            status_.Insert(detail::kNextStatement);
          }
        }
      }
    }

    if (!dead) {
      status_.ResolveJump(stmt);
    }
  }

  void Visit(ThrowStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    if (!dead) {
      status_.Kill();
    }

    stmt->set_normal(normal_);
  }

  void Visit(TryStatement* stmt) {
    // TODO(Constellation)
    // check try main block is exception-safe or not.
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    Block* catch_block =
        (stmt->catch_block()) ? stmt->catch_block().Address() : NULL;
    Block* finally_block =
        (stmt->finally_block()) ? stmt->finally_block().Address() : NULL;

    stmt->body()->Accept(this);

    if (catch_block) {
      // catch
      if (!dead) {
        if (IsDeadStatement()) {
          status_.Insert(detail::kNextStatement);
        } else {
          status_.Insert(stmt);
        }
      }
      catch_block->Accept(this);
      if (!dead) {
        if (status_.Has(stmt)) {
          status_.Erase(stmt);
          if (IsDeadStatement()) {
            status_.Insert(detail::kNextStatement);
          }
        }
      }
    }

    if (finally_block) {
      // finally
      // unless try statement is deadcode,
      // finally block is always live code.
      if (!dead) {
        if (IsDeadStatement()) {
          status_.Insert(detail::kNextStatement);
        } else {
          status_.Insert(stmt);
        }
      }
      finally_block->Accept(this);
      if (!dead) {
        if (status_.Has(stmt)) {
          status_.Erase(stmt);
        } else {
          if (!IsDeadStatement()) {
            status_.Kill();
          }
        }
      }
    }

    stmt->set_normal(normal_);
  }

  void Visit(DebuggerStatement* stmt) {
    CheckDeadStatement(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(ExpressionStatement* stmt) {
    CheckDeadStatement(stmt);
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

  class ExecutionSwitcher : private iv::core::Noncopyable<ExecutionSwitcher> {
   public:
    ExecutionSwitcher(Analyzer* analyzer, std::shared_ptr<ExecutionContext> ctx)
      : analyzer_(analyzer),
        prev_(analyzer->context()) {
      analyzer_->set_context(ctx);
    }

    ~ExecutionSwitcher() {
      analyzer_->set_context(prev_);
    }

   private:
    Analyzer* analyzer_;
    std::shared_ptr<ExecutionContext> prev_;
  };

  void Visit(FunctionLiteral* literal) {
    std::shared_ptr<ExecutionContext> ctx = context_->Create(literal);
    AnalyzeFunctionLiteral(literal, ctx);
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

  std::shared_ptr<ExecutionContext> context() const {
    return context_;
  }

  void set_context(std::shared_ptr<ExecutionContext> ctx) {
    context_ = ctx;
  }

  // remember this variable is located at this function stack
  void StoreVariable(Identifier* ident, VariableType type = VARIABLE_STACK) {
    // const iv::core::UString key(ident->value().begin(), ident->value().end());
  }

  bool CheckDeadStatement(const Statement* stmt) {
    if (IsDeadStatement()) {
      reporter_->ReportDeadStatement(*stmt);
      return true;
    } else {
      return false;
    }
  }

  bool IsDeadStatement() const {
    return status_.IsDeadStatement();
  }

  Statement* normal_;
  Statement* raised_;
  std::shared_ptr<ExecutionContext> context_;
  Reporter* reporter_;
  ContinuationStatus status_;
};

template<typename Source, typename Reporter>
inline void Analyze(FunctionLiteral* global, const Source& src, Reporter* reporter) {
  Analyzer<Reporter> analyzer(reporter);
  analyzer.Analyze(global);
}

}  // namespace az
#endif  // _AZ_ANALYZER_H_
