// Az Analyzer
// rewrite AST and analyze stack and heap variables status
#ifndef _AZ_ANALYZER_H_
#define _AZ_ANALYZER_H_
#include <iv/ast_visitor.h>
#include <iv/utils.h>
#include <iv/maybe.h>
#include <iv/noncopyable.h>
#include <iv/unicode.h>
#include <iv/ustringpiece.h>
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
    std::shared_ptr<Environment> target =
        context_->GetLexicalEnvironment()->Lookup(literal->name().Address()->value());

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
    CheckAutomaticSemicolonInsertion(var);
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
    stmt->body()->Accept(this);

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
    CheckAutomaticSemicolonInsertion(stmt);
    const bool dead = IsDeadStatement();

    if (!dead) {
      status_.JumpTo(stmt->target());
    }

    stmt->set_normal(stmt->target());
  }

  void Visit(BreakStatement* stmt) {
    CheckDeadStatement(stmt);
    CheckAutomaticSemicolonInsertion(stmt);
    const bool dead = IsDeadStatement();

    if (!dead) {
      status_.JumpTo(stmt->target());
    }

    stmt->set_normal(stmt->target());
  }

  void Visit(ReturnStatement* stmt) {
    CheckDeadStatement(stmt);
    CheckAutomaticSemicolonInsertion(stmt);
    const bool dead = IsDeadStatement();

    if (!dead) {
      status_.Kill();
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
    CheckAutomaticSemicolonInsertion(stmt);
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
      context_->GetLexicalEnvironment()->Instantiate(stmt->catch_name().Address()->value());
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
    CheckAutomaticSemicolonInsertion(stmt);
    stmt->set_normal(normal_);
  }

  void Visit(ExpressionStatement* stmt) {
    CheckDeadStatement(stmt);
    CheckAutomaticSemicolonInsertion(stmt);
    stmt->set_normal(normal_);

    stmt->expr()->Accept(this);
  }

  void Visit(Assignment* assign) {
  }

  void Visit(BinaryOperation* binary) {
    using iv::core::Token;
    const Token::Type token = binary->op();
    switch (token) {
      case Token::LOGICAL_AND: {  // &&
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // TODO(Constellation) multiple type
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = rhs;
        break;
      }

      case Token::LOGICAL_OR: {  // ||
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = lhs;
        break;
      }

      case Token::ADD: {  // +
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;

        if (lhs == TYPE_STRING || rhs == TYPE_STRING) {
          // allow String + Any
          type_ = TYPE_STRING;
        } else {
          // not allow Number + Any(except String)
          if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
            reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
          }
          if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
            reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
          }
          type_ = TYPE_NUMBER;
        }
        break;
      }

      case Token::SUB: {  // -
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;

        // TODO(Constellation)
        // "1000" - 0 is allowed?
        // this case is allowed mode
        if (lhs != TYPE_NUMBER && lhs != TYPE_STRING && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_STRING && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::SHR: {  // >>>
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::SAR: {  // >>
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::SHL: {  // <<
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::MUL: {  // *
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::DIV: {  // /
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::MOD: {  // %
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::LT: {  // <
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::GT: {  // >
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::LTE: {  // <=
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::GTE: {  // >=
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::INSTANCEOF: {  // instanceof
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // TODO(Constellation) special instanceof type check
        if (!IsObjectType(lhs) && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_OBJECT);
        }
        if (!IsObjectType(rhs) && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_OBJECT);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::IN: {  // in
        binary->left()->Accept(this);
        // const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (!IsObjectType(rhs) && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_OBJECT);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::EQ: {  // ==
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::NE: {  // !=
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        // Array like Object type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::EQ_STRICT: {  // ===
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::NE_STRICT: {  // !==
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        // same type
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, rhs);
        }
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::BIT_AND: {  // &
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::BIT_XOR: {  // ^
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::BIT_OR: {  // |
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != TYPE_NUMBER && lhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, lhs, TYPE_NUMBER);
        }
        if (rhs != TYPE_NUMBER && rhs != TYPE_ANY) {
          reporter_->ReportTypeConflict(*binary, rhs, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::COMMA: {  // ,
        binary->left()->Accept(this);
        const JSType lhs = type_;
        binary->right()->Accept(this);
        const JSType rhs = type_;
        if (lhs != rhs && lhs != TYPE_ANY && rhs != TYPE_ANY) {
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
  }

  void Visit(UnaryOperation* unary) {
    using iv::core::Token;
    switch (unary->op()) {
      case Token::DELETE: {
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
        type_ = TYPE_BOOLEAN;
        break;
      }

      case Token::VOID: {
        unary->expr()->Accept(this);
        type_ = TYPE_UNDEFINED;
        break;
      }

      case Token::TYPEOF: {
        unary->expr()->Accept(this);
        type_ = TYPE_STRING;
        break;
      }

      case Token::INC: {
        unary->expr()->Accept(this);
        if (type_ != TYPE_NUMBER && type_ != TYPE_ANY) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::DEC: {
        unary->expr()->Accept(this);
        if (type_ != TYPE_NUMBER && type_ != TYPE_ANY) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::ADD: {
        unary->expr()->Accept(this);
        if (type_ != TYPE_NUMBER && type_ != TYPE_ANY) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::SUB: {
        unary->expr()->Accept(this);
        if (type_ != TYPE_NUMBER && type_ != TYPE_ANY) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::BIT_NOT: {
        unary->expr()->Accept(this);
        if (type_ != TYPE_NUMBER && type_ != TYPE_ANY) {
          reporter_->ReportTypeConflict(*unary, type_, TYPE_NUMBER);
        }
        type_ = TYPE_NUMBER;
        break;
      }

      case Token::NOT: {
        unary->expr()->Accept(this);
        type_ = TYPE_BOOLEAN;
        break;
      }

      default:
        UNREACHABLE();
    }
  }

  void Visit(PostfixExpression* postfix) {
  }

  void Visit(StringLiteral* literal) {
    // type resolve
    type_ = TYPE_STRING;
  }

  void Visit(NumberLiteral* literal) {
    // type resolve
    type_ = TYPE_NUMBER;
  }

  void Visit(Identifier* literal) {
  }

  void Visit(ThisLiteral* literal) {
    // type resolve
    type_ = TYPE_USER_OBJECT;
  }

  void Visit(NullLiteral* lit) {
    type_ = TYPE_NULL;
  }

  void Visit(TrueLiteral* lit) {
    type_ = TYPE_BOOLEAN;
  }

  void Visit(FalseLiteral* lit) {
    type_ = TYPE_BOOLEAN;
  }

  void Visit(RegExpLiteral* literal) {
    type_ = TYPE_REGEXP;
  }

  void Visit(ArrayLiteral* literal) {
    type_ = TYPE_ARRAY;
  }

  void Visit(ObjectLiteral* literal) {
    type_ = TYPE_OBJECT;
  }

  void Visit(FunctionLiteral* literal) {
    std::shared_ptr<ExecutionContext> ctx = context_->Create(literal);
    AnalyzeFunctionLiteral(literal, ctx);
    type_ = TYPE_FUNCTION;
  }

  void Visit(IdentifierAccess* prop) {
  }

  void Visit(IndexAccess* prop) {
  }

  void Visit(FunctionCall* call) {
    type_ = TYPE_ANY;
  }

  void Visit(ConstructorCall* call) {
    type_ = TYPE_USER_OBJECT;
  }

  void Visit(Declaration* decl) {
    Var& ref = context_->GetVariableEnvironment()->Get(decl->name()->value());
    if (ref.IsDeclared()) {
      // duplicate declaration
      reporter_->ReportDuplicateDeclaration(*decl);
    }
    ref.AddDeclaration(decl);
    if (const iv::core::Maybe<Expression> expr = decl->expr()) {
      JSType type = ResolveType(expr.Address());
      if (ref.IsType(type)) {
        ref.InsertType(type);
      } else {
        reporter_->ReportTypeConflict(*decl, ref.GetPrimaryType(), type);
      }
    }
  }

  void Visit(CaseClause* clause) {
  }

  std::shared_ptr<ExecutionContext> context() const {
    return context_;
  }

  void set_context(std::shared_ptr<ExecutionContext> ctx) {
    context_ = ctx;
  }

  // remember this variable is located at this function stack
  bool InstantiateVariable(Identifier* ident, VariableType type = VARIABLE_STACK) {
    return context_->GetVariableEnvironment()->Instantiate(ident->value());
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

  JSType ResolveType(Expression* expr) {
    type_ = TYPE_UNDEFINED;
    expr->Accept(this);
    return type_;
  }

  void CheckAutomaticSemicolonInsertion(Statement* stmt) {
    //  VariableStatement
    //  ContinueStatement
    //  BreakStatement
    //  ReturnStatement
    //  ThrowStatement
    //  DebuggerStatement
    //  ExpressionStatement
    if (src_[stmt->end_position() - 1] != ';') {
      // not ends with semicolon
      reporter_->ReportAutomaticSemicolonInsertion(*stmt);
    }
  }

  const Source& src_;
  Statement* normal_;
  Statement* raised_;
  std::shared_ptr<ExecutionContext> context_;
  Reporter* reporter_;
  ContinuationStatus status_;
  JSType type_;
};

template<typename Source, typename Reporter>
inline void Analyze(FunctionLiteral* global, const Source& src, Reporter* reporter) {
  Analyzer<Source, Reporter> analyzer(src, reporter);
  analyzer.Analyze(global);
}

}  // namespace az
#endif  // _AZ_ANALYZER_H_
