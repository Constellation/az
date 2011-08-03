// Az Analyzer
// rewrite AST and analyze stack and heap variables status
#ifndef _AZ_ANALYZER_H_
#define _AZ_ANALYZER_H_
#include <iostream>
#include <iv/ast_visitor.h>
#include <iv/utils.h>
#include <iv/maybe.h>
#include <iv/noncopyable.h>
#include <iv/unicode.h>
#include <iv/ustringpiece.h>
#include <az/ast_fwd.h>
#include <az/factory.h>
#include <az/environment.h>
#include <az/execution_context.h>

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

template<typename Source, typename Reporter>
class Analyzer
  : public iv::core::ast::AstVisitor<AstFactory>::type,
    private iv::core::Noncopyable<Analyzer<Source, Reporter> > {
 public:

  Analyzer(const Source& src, Reporter* reporter)
    : src_(src),
      normal_(NULL),
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
        if (InstantiateVariable((*it)->name().Address())) {
          // variable name is duplicate
          // TODO(Constellation) insert report
        }
        // because this expr is Function Declaration, so instantiate as
        // TYPE_FUNCTION
        const Symbol target = Intern((*it)->name().Address()->value());
        Var& ref = context_->GetVariableEnvironment()->Get(target);
        ref.InsertType(AType(TYPE_FUNCTION));
        ref.AddDeclaration(*it);
      }
    }
    {
      // variables
      typedef Scope::Variables Variables;
      const Variables& vars = scope.variables();
      for (Variables::const_iterator it = vars.begin(),
           last = vars.end(); it != last; ++it) {
        if (InstantiateVariable(it->first)) {
          // variable name is duplicate
          // TODO(Constellation) insert report
        }
      }
    }
    AnalyzeStatements(literal->body());

    if (!status_.IsDeadStatement() && !literal->body().empty()) {
      // last statement is not dead and return type is exists,
      if (context_->IsReturnedWithValue()) {
        reporter_->ReportNotProcedure(*literal->body().back());
      }
    }
    context_->ReportNotProcedure(reporter_);
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
    reporter_->ReportFunctionStatement(*stmt);

    stmt->set_normal(normal_);

    // variable analyze
    FunctionLiteral* literal = stmt->function();

    // the name is trapped, get environment.
    const Symbol target = Intern(literal->name().Address()->value());
    context_->GetLexicalEnvironment()->Lookup(target);

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

    const Declarations& decls = var->decls();
    for (Declarations::const_iterator it = decls.begin(),
         last = decls.end(); it != last; ++it) {
      Visit(*it);
    }
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
      if (IsDeadStatement()) {
        status_.Insert(detail::kNextStatement);
      }
    }
  }

  void Visit(WhileStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    stmt->set_normal(stmt->body());
    stmt->body()->Accept(this);

    if (!dead) {
      status_.ResolveJump(stmt);
      if (IsDeadStatement()) {
        status_.Insert(detail::kNextStatement);
      }
    }
  }

  void Visit(ForStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    stmt->set_normal(stmt->body());
    if (stmt->init()) {
      if (VariableStatement* var = stmt->init().Address()->AsVariableStatement()) {
        Visit(var->decls().front());
        const Symbol name = Intern(var->decls().front()->name()->value());
        Var& ref = context_->GetVariableEnvironment()->Get(name);
        ref.InsertType(AType(TYPE_ANY));
      } else {
        stmt->init().Address()->AsExpressionStatement()->expr()->Accept(this);
      }
    }
    if (stmt->cond()) {
      stmt->cond().Address()->Accept(this);
    }
    stmt->body()->Accept(this);
    if (stmt->next()) {
      stmt->next().Address()->Accept(this);
    }

    if (!dead) {
      status_.ResolveJump(stmt);
      if (IsDeadStatement()) {
        status_.Insert(detail::kNextStatement);
      }
    }
  }

  void Visit(ForInStatement* stmt) {
    CheckDeadStatement(stmt);
    const bool dead = IsDeadStatement();

    stmt->set_normal(stmt->body());
    if (VariableStatement* var = stmt->each()->AsVariableStatement()) {
      Visit(var->decls().front());
      const Symbol name = Intern(var->decls().front()->name()->value());
      Var& ref = context_->GetVariableEnvironment()->Get(name);
      ref.InsertType(AType(TYPE_ANY));
    } else {
      stmt->each()->AsExpressionStatement()->expr()->Accept(this);
    }
    stmt->enumerable()->Accept(this);
    stmt->body()->Accept(this);

    if (!dead) {
      status_.ResolveJump(stmt);
      if (IsDeadStatement()) {
        status_.Insert(detail::kNextStatement);
      }
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
    if (stmt->expr()) {
      stmt->expr().Address()->Accept(this);
      if (!context_->InsertReturnType(type_)) {
        if (!type_.IsVacantType()) {
          reporter_->ReportTypeConflict(*stmt, context_->GetReturnType(), type_);
        }
      }
    } else {
      context_->AddProceduralReturn(stmt);
    }
    stmt->set_normal(normal_);
  }

  void Visit(WithStatement* stmt) {
    CheckDeadStatement(stmt);
    stmt->set_normal(stmt->body());

    // TODO(Constellation) use RAII
    context_->DownInWithStatement(stmt);
    stmt->body()->Accept(this);
    context_->UpFromWithStatement();
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
    bool has_default_clause = false;
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
      if ((*it)->IsDefault()) {
        has_default_clause = true;
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
      if (IsDeadStatement() && !has_default_clause) {
        status_.Insert(detail::kNextStatement);
      }
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
      context_->DownInCatchBlock(stmt);
      // catch name instantiation
      const Symbol name = Intern(stmt->catch_name().Address()->value());
      context_->GetLexicalEnvironment()->Instantiate(name);
      catch_block->Accept(this);
      context_->UpFromCatchBlock();
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

    stmt->expr()->Accept(this);
  }

  void Visit(Assignment* assign) {
    Identifier* ident = assign->left()->AsIdentifier();
    std::shared_ptr<Environment> env;
    bool variable_found = false;
    if (ident) {
      const Symbol name = Intern(ident->value());
      env = context_->GetLexicalEnvironment()->Lookup(name);
      Environment::TrapStatus stat = env->IsTrapped(name);
      if (stat == Environment::VARIABLE_FOUND) {
        type_ = env->Get(name).GetType();
        variable_found = true;
      } else if (stat == Environment::VARIABLE_NOT_FOUND) {
        // implicit global
        reporter_->ReportLookupImplicitGlobalVariable(*ident);
        type_ = AType();
      } else {
        type_ = AType();
      }
    } else {
      assign->left()->Accept(this);
    }
    const AType lhs = type_;
    assign->right()->Accept(this);
    const AType rhs = type_;
    if (!IsConservativeEqualType(lhs, rhs)) {
      reporter_->ReportTypeConflict(*assign, lhs, rhs);
    }
    if (variable_found) {
      env->Get(Intern(ident->value())).InsertType(rhs);
    }
    type_ = rhs;
  }

  void Visit(BinaryOperation* binary) {
    using iv::core::Token;
    const Token::Type token = binary->op();
    switch (token) {
      case Token::TK_LOGICAL_AND: {  // &&
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // TODO(Constellation) multiple type
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = rhs;
        break;
      }

      case Token::TK_LOGICAL_OR: {  // ||
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = lhs;
        break;
      }

      case Token::TK_ADD: {  // +
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;

        if (lhs.IsPrimaryTyped(TYPE_STRING) ||
            rhs.IsPrimaryTyped(TYPE_STRING)) {
          // allow String + Any
          type_ = AType(TYPE_STRING);
        } else {
          // not allow Number + Any(except String)
          if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
            reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
          }
          if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
            reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
          }
          type_ = AType(TYPE_NUMBER);
        }
        break;
      }

      case Token::TK_SUB: {  // -
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;

        // TODO(Constellation)
        // "1000" - 0 is allowed?
        // this case is allowed mode
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) &&
            !lhs.IsPrimaryTyped(TYPE_STRING) &&
            !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) &&
            !rhs.IsPrimaryTyped(TYPE_STRING) &&
            !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_SHR: {  // >>>
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_SAR: {  // >>
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_SHL: {  // <<
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_MUL: {  // *
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_DIV: {  // /
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_MOD: {  // %
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_LT: {  // <
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_GT: {  // >
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_LTE: {  // <=
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_GTE: {  // >=
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_INSTANCEOF: {  // instanceof
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // TODO(Constellation) special instanceof type check
        if (!lhs.IsObjectType() && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_OBJECT);
        }
        if (!rhs.IsObjectType() && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_OBJECT);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_IN: {  // in
        binary->left()->Accept(this);
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!rhs.IsObjectType() && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_OBJECT);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_EQ: {  // ==
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_NE: {  // !=
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        // Array like Object type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_EQ_STRICT: {  // ===
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_NE_STRICT: {  // !==
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        // same type
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_BIT_AND: {  // &
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_BIT_XOR: {  // ^
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_BIT_OR: {  // |
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!lhs.IsPrimaryTyped(TYPE_NUMBER) && !lhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (!rhs.IsPrimaryTyped(TYPE_NUMBER) && !rhs.IsVacantType()) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_COMMA: {  // ,
        binary->left()->Accept(this);
        const AType lhs = type_;
        binary->right()->Accept(this);
        const AType rhs = type_;
        if (!IsConservativeEqualType(lhs, rhs)) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = rhs;
        break;
      }

      default:
        UNREACHABLE();
    }
  }

  void Visit(ConditionalExpression* cond) {
    cond->cond()->Accept(this);
    cond->left()->Accept(this);
    const AType lhs = type_;
    cond->right()->Accept(this);
    const AType rhs = type_;
    if (!IsConservativeEqualType(lhs, rhs)) {
      reporter_->ReportTypeConflict(*cond, lhs, rhs);
    }
    type_ = AType::Merged(lhs, rhs);
  }

  void Visit(UnaryOperation* unary) {
    using iv::core::Token;
    switch (unary->op()) {
      case Token::TK_DELETE: {
        Expression* expr = unary->expr();
        if (expr->IsValidLeftHandSide()) {
          // Identifier
          // PropertyAccess
          // FunctionCall
          // ConstructorCall
          expr->Accept(this);
          if (expr->AsIdentifier()) {
            // delete Identifier
            reporter_->ReportDeleteToIdentifier(*unary);
          } else if (!expr->AsPropertyAccess()) {
            // delete FunctionCall / delete ConstructorCall
            reporter_->ReportDeleteToCallResult(*unary);
          }
        } else {
          // delete "OK" etc...
          expr->Accept(this);
          reporter_->ReportDeleteToInvalidLHS(*unary);
        }
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      case Token::TK_VOID: {
        unary->expr()->Accept(this);
        type_ = AType(TYPE_UNDEFINED);
        break;
      }

      case Token::TK_TYPEOF: {
        unary->expr()->Accept(this);
        type_ = AType(TYPE_STRING);
        break;
      }

      case Token::TK_INC: {
        if (unary->expr()->AsCall()) {
          reporter_->ReportIncrementToCallResult(*unary);
        }
        unary->expr()->Accept(this);
        if (type_.IsPrimaryTyped(TYPE_NUMBER) &&
            type_.IsVacantType()) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_DEC: {
        if (unary->expr()->AsCall()) {
          reporter_->ReportDecrementToCallResult(*unary);
        }
        unary->expr()->Accept(this);
        if (type_.IsPrimaryTyped(TYPE_NUMBER) &&
            type_.IsVacantType()) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_ADD: {
        unary->expr()->Accept(this);
        if (type_.IsPrimaryTyped(TYPE_NUMBER) &&
            type_.IsVacantType()) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_SUB: {
        unary->expr()->Accept(this);
        if (type_.IsPrimaryTyped(TYPE_NUMBER) &&
            type_.IsVacantType()) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_BIT_NOT: {
        unary->expr()->Accept(this);
        if (type_.IsPrimaryTyped(TYPE_NUMBER) &&
            type_.IsVacantType()) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = AType(TYPE_NUMBER);
        break;
      }

      case Token::TK_NOT: {
        unary->expr()->Accept(this);
        type_ = AType(TYPE_BOOLEAN);
        break;
      }

      default:
        UNREACHABLE();
    }
  }

  void Visit(PostfixExpression* postfix) {
    using iv::core::Token;
    const Token::Type token = postfix->op();
    Expression* expr = postfix->expr();
    if (expr->AsCall()) {
      if (token == Token::TK_INC) {
        reporter_->ReportPostfixIncrementToCallResult(*postfix);
      } else {
        reporter_->ReportPostfixDecrementToCallResult(*postfix);
      }
    }
    expr->Accept(this);
    if (type_.IsPrimaryTyped(TYPE_NUMBER) && type_.IsVacantType()) {
      reporter_->ReportTypeConflict(*postfix, type_, TYPE_NUMBER);
    }
    type_ = AType(TYPE_NUMBER);
  }

  void Visit(StringLiteral* literal) {
    // type resolve
    type_ = AType(TYPE_STRING);
  }

  void Visit(NumberLiteral* literal) {
    // type resolve
    type_ = AType(TYPE_NUMBER);
  }

  void Visit(Identifier* literal) {
    const Symbol name = Intern(literal->value());
    std::shared_ptr<Environment> env =
        context_->GetLexicalEnvironment()->Lookup(name);
    Environment::TrapStatus stat = env->IsTrapped(name);
    if (stat == Environment::VARIABLE_FOUND) {
      Var& var = env->Get(name);
      if (!var.IsDeclared()) {
        reporter_->ReportLookupNotDeclaredVariable(*literal);
      }
      type_ = var.GetType();
    } else if (stat == Environment::TRAP_ONLY) {
      // implicit global
      if (env->IsGlobal()) {
        reporter_->ReportLookupImplicitGlobalVariable(*literal);
      }
      type_ = AType(TYPE_ANY);
    }
  }

  void Visit(ThisLiteral* literal) {
    // type resolve
    type_ = AType(TYPE_OBJECT);
  }

  void Visit(NullLiteral* lit) {
    type_ = AType(TYPE_NULL);
  }

  void Visit(TrueLiteral* lit) {
    type_ = AType(TYPE_BOOLEAN);
  }

  void Visit(FalseLiteral* lit) {
    type_ = AType(TYPE_BOOLEAN);
  }

  void Visit(RegExpLiteral* literal) {
    type_ = AType(TYPE_REGEXP);
  }

  void Visit(ArrayLiteral* literal) {
    type_ = AType(TYPE_ARRAY);
  }

  void Visit(ObjectLiteral* literal) {
    type_ = AType(TYPE_OBJECT);
  }

  void Visit(FunctionLiteral* literal) {
    std::shared_ptr<ExecutionContext> ctx = context_->Create(literal);
    AnalyzeFunctionLiteral(literal, ctx);
    type_ = AType(TYPE_FUNCTION);
  }

  void Visit(IdentifierAccess* prop) {
    prop->target()->Accept(this);
    if (!type_.IsVacantType() &&
        (type_.IsPrimaryTyped(TYPE_UNDEFINED) || type_.IsPrimaryTyped(TYPE_NULL))) {
      reporter_->ReportIdentifierAccessToNotObjectType(*prop, type_);
    }
    // TODO(Constellation) fix
    type_ = AType();
  }

  void Visit(IndexAccess* prop) {
    prop->target()->Accept(this);
    if (!type_.IsVacantType() &&
        (type_.IsPrimaryTyped(TYPE_UNDEFINED) || type_.IsPrimaryTyped(TYPE_NULL))) {
      reporter_->ReportIndexAccessToNotObjectType(*prop, type_);
    }
    prop->key()->Accept(this);
    if (!type_.IsVacantType() &&
        (!type_.IsPrimaryTyped(TYPE_STRING) && !type_.IsPrimaryTyped(TYPE_NUMBER))) {
      reporter_->ReportIndexKeyIsNotStringOrNumber(*prop, type_);
    }
    // TODO(Constellation) fix
    type_ = AType();
  }

  void Visit(FunctionCall* call) {
    call->target()->Accept(this);
    if (!type_.IsVacantType() && !type_.IsPrimaryTyped(TYPE_FUNCTION)) {
      reporter_->ReportCallToNotFunction(*call, type_);
    }
    for (Expressions::const_iterator it = call->args().begin(),
         last = call->args().end(); it != last; ++it) {
      (*it)->Accept(this);
    }
    type_ = AType();
  }

  void Visit(ConstructorCall* call) {
    call->target()->Accept(this);
    if (!type_.IsVacantType() && !type_.IsPrimaryTyped(TYPE_FUNCTION)) {
      reporter_->ReportConstructToNotFunction(*call, type_);
    }
    for (Expressions::const_iterator it = call->args().begin(),
         last = call->args().end(); it != last; ++it) {
      (*it)->Accept(this);
    }
    type_ = AType(TYPE_OBJECT);
  }

  void Visit(Declaration* decl) {
    const Symbol name = Intern(decl->name()->value());
    Var& ref = context_->GetVariableEnvironment()->Get(name);
    if (ref.IsDeclared()) {
      // duplicate declaration
      reporter_->ReportDuplicateDeclaration(*decl);
    }
    ref.AddDeclaration(decl);
    if (const iv::core::Maybe<Expression> expr = decl->expr()) {
      const AType type = ResolveType(expr.Address());
      if (IsConservativeEqualType(ref.GetType(), type)) {
        ref.InsertType(type);
      } else {
        reporter_->ReportTypeConflict(*decl, ref.GetType(), type);
        ref.InsertType(type);
      }
    }
  }

  void Visit(CaseClause* dummy) { }

  std::shared_ptr<ExecutionContext> context() const {
    return context_;
  }

  void set_context(std::shared_ptr<ExecutionContext> ctx) {
    context_ = ctx;
  }

  // remember this variable is located at this function stack
  bool InstantiateVariable(Identifier* ident, VariableType type = VARIABLE_STACK) {
    return context_->GetVariableEnvironment()->Instantiate(Intern(ident->value()));
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

  AType ResolveType(Expression* expr) {
    type_ = AType(TYPE_UNDEFINED);
    expr->Accept(this);
    return type_;
  }

  const Source& src_;
  Statement* normal_;
  Statement* raised_;
  std::shared_ptr<ExecutionContext> context_;
  Reporter* reporter_;
  ContinuationStatus status_;
  AType type_;
};

template<typename Source, typename Reporter>
inline void Analyze(FunctionLiteral* global, const Source& src, Reporter* reporter) {
  Analyzer<Source, Reporter> analyzer(src, reporter);
  analyzer.Analyze(global);
}

}  // namespace az
#endif  // _AZ_ANALYZER_H_
