// resolve which identifier is STACK or HEAP
#ifndef _AZ_CFA2_BINDING_RESOLVER_H_
#define _AZ_CFA2_BINDING_RESOLVER_H_
#include <functional>
#include <iv/noncopyable.h>
#include <iv/maybe.h>
#include <az/ast_fwd.h>
#include <az/symbol.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/binding_resolver_fwd.h>
namespace az {
namespace cfa2 {

// Statements

void BindingResolver::Resolve(FunctionLiteral* global) {
  Visit(global);
}

void BindingResolver::Visit(Block* block) {
  block->set_raised(raised_);
  block->set_jump_to(normal_);
  if (block->body().empty()) {
    // no Statement, so next is normal
    block->set_normal(normal_);
  } else {
    // first Statement
    block->set_normal(block->body().front());
  }
  MarkStatements(block->body());
}

void BindingResolver::MarkStatements(const Statements& body) {
  Statement* normal = normal_;
  for (Statements::const_iterator it = body.begin(),
       last = body.end(); it != last; ++it) {
    Statements::const_iterator next = it;
    std::advance(next, 1);
    if (next != last) {
      normal_ = *next;
    } else {
      normal_ = normal;
    }
    (*it)->Accept(this);
  }
}

void BindingResolver::Visit(FunctionStatement* func) {
  func->set_normal(normal_);
  func->set_raised(raised_);
  func->function()->Accept(this);
}

void BindingResolver::Visit(FunctionDeclaration* func) {
  // nothing
  func->set_normal(normal_);
  func->set_raised(raised_);
}

void BindingResolver::Visit(VariableStatement* var) {
  var->set_normal(normal_);
  var->set_raised(raised_);
  for (Declarations::const_iterator it = var->decls().begin(),
       last = var->decls().end(); it != last; ++it) {
    if (const iv::core::Maybe<Expression> expr = (*it)->expr()) {
      expr.Address()->Accept(this);
    }
  }
}

void BindingResolver::Visit(EmptyStatement* stmt) {
  // not have Identifier to value
  stmt->set_normal(normal_);
  stmt->set_raised(raised_);
}

void BindingResolver::Visit(IfStatement* stmt) {
  stmt->set_normal(stmt->then_statement());
  stmt->set_raised(raised_);
  stmt->cond()->Accept(this);

  Statement* normal = normal_;
  if (const iv::core::Maybe<Statement> target = stmt->else_statement()) {
    normal_ = target.Address();
  }
  stmt->then_statement()->Accept(this);
  normal_ = normal;
  if (const iv::core::Maybe<Statement> target = stmt->else_statement()) {
    target.Address()->Accept(this);
  }
}

void BindingResolver::Visit(DoWhileStatement* stmt) {
  stmt->set_normal(stmt->body());
  stmt->set_raised(raised_);
  stmt->set_jump_to(normal_);
  Statement* previous_normal = normal_;
  ExpressionStatement* cond = heap_->NewWrappedStatement(stmt->cond());
  normal_ = cond;
  stmt->body()->Accept(this);
  normal_ = previous_normal;
  Visit(cond);
}

void BindingResolver::Visit(WhileStatement* stmt) {
  stmt->set_normal(stmt->body());
  stmt->set_raised(raised_);
  stmt->set_jump_to(normal_);
  stmt->cond()->Accept(this);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(ForStatement* stmt) {
  stmt->set_raised(raised_);
  stmt->set_jump_to(normal_);

  Statement* previous_normal = normal_;
  normal_ = stmt->body();
  if (const iv::core::Maybe<Statement> init = stmt->init()) {
    normal_ = init.Address();
  }
  stmt->set_normal(normal_);

  ExpressionStatement* cond = NULL;
  normal_ = stmt->body();
  if (const iv::core::Maybe<Expression> conde = stmt->cond()) {
    cond = heap_->NewWrappedStatement(conde.Address());
    normal_ = cond;
  }

  // now, normal is cond / body
  if (const iv::core::Maybe<Statement> init = stmt->init()) {
    init.Address()->Accept(this);
  }

  normal_ = stmt->body();

  // now, normal is body
  if (cond) {
    Visit(cond);
  }

  normal_ = previous_normal;
  ExpressionStatement* next = NULL;
  if (const iv::core::Maybe<Expression> nexte = stmt->next()) {
    next = heap_->NewWrappedStatement(nexte.Address());
    normal_ = next;
  }

  // now, normal is next / previous_normal
  stmt->body()->Accept(this);

  normal_ = previous_normal;

  if (next) {
    Visit(next);
  }

  stmt->set_cond_statement(cond);
  stmt->set_next_statement(next);
}

void BindingResolver::Visit(ForInStatement* stmt) {
  stmt->set_raised(raised_);
  stmt->set_normal(stmt->body());
  stmt->set_jump_to(normal_);
  stmt->each()->Accept(this);
  stmt->enumerable()->Accept(this);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(ContinueStatement* stmt) {
  // not have Identifier to value
  stmt->set_normal(stmt->target()->jump_to());
  stmt->set_raised(raised_);
}

void BindingResolver::Visit(BreakStatement* stmt) {
  // not have Identifier to value
  if (!stmt->target() && stmt->label()) {
    // like
    //   do {
    //     self: break self;
    //   } while (false)
    // 
    // interpret this as EmptyStatement
    stmt->set_normal(normal_);
    stmt->set_raised(raised_);
  } else {
    stmt->set_normal(stmt->target()->jump_to());
    stmt->set_raised(raised_);
  }
}

void BindingResolver::Visit(ReturnStatement* stmt) {
  stmt->set_normal(normal_);
  stmt->set_raised(raised_);
  if (const iv::core::Maybe<Expression> expr = stmt->expr()) {
    expr.Address()->Accept(this);
  }
}

void BindingResolver::Visit(WithStatement* stmt) {
  stmt->set_normal(stmt->body());
  stmt->set_raised(raised_);
  // this is ambiguous, but use this
  stmt->context()->Accept(this);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(LabelledStatement* stmt) {
  stmt->set_normal(stmt->body());
  stmt->set_raised(raised_);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(SwitchStatement* stmt) {
  typedef SwitchStatement::CaseClauses CaseClauses;
  const CaseClauses& clauses = stmt->clauses();
  stmt->set_raised(raised_);
  stmt->set_jump_to(normal_);
  if (clauses.empty()) {
    stmt->set_normal(normal_);
  } else {
    stmt->set_normal(clauses.front());
  }
  Statement* normal = normal_;
  stmt->expr()->Accept(this);

  for (CaseClauses::const_iterator it = clauses.begin(),
       last = clauses.end(); it != last; ++it) {
    CaseClauses::const_iterator next = it;
    std::advance(next, 1);
    if (next != last) {
      normal_ = *next;
    } else {
      normal_ = normal;
    }
    Visit(*it);
  }
}

void BindingResolver::Visit(CaseClause* clause) {
  clause->set_raised(raised_);
  if (clause->body().empty()) {
    // no Statement, so next is normal
    clause->set_normal(normal_);
  } else {
    // first Statement
    clause->set_normal(clause->body().front());
  }
  if (const iv::core::Maybe<Expression> expr = clause->expr()) {
    expr.Address()->Accept(this);
  }
  MarkStatements(clause->body());
}

void BindingResolver::Visit(ThrowStatement* stmt) {
  stmt->set_normal(normal_);
  stmt->set_raised(raised_);
  stmt->expr()->Accept(this);
}

void BindingResolver::Visit(TryStatement* stmt) {
  stmt->set_normal(stmt->body());
  stmt->set_raised(raised_);

  Statement* raised = raised_;
  Statement* normal = normal_;
  if (const iv::core::Maybe<Block> block = stmt->finally_block()) {
    // normal and raised to Finally Block
    raised_ = normal_ = block.Address();
  }

  Statement* catch_raised = raised_;
  if (const iv::core::Maybe<Block> block = stmt->catch_block()) {
    raised_ = stmt;  // set TryStatement as catch
  }

  stmt->body()->Accept(this);

  if (const iv::core::Maybe<Block> block = stmt->catch_block()) {
    raised_ = catch_raised;
    Identifier* ident = stmt->catch_name().Address();
    const Symbol name = Intern(ident->value());
    Binding* binding = heap_->Instantiate(name);
    ident->set_refer(binding);
    inner_scope_->push_back(binding);
    Visit(block.Address());
    inner_scope_->pop_back();
  }

  normal_ = normal;
  raised_ = raised;
  if (const iv::core::Maybe<Block> block = stmt->finally_block()) {
    Visit(block.Address());
  }
}

void BindingResolver::Visit(DebuggerStatement* stmt) {
  stmt->set_normal(normal_);
  stmt->set_raised(raised_);
  // not have Identifier to value
}

void BindingResolver::Visit(ExpressionStatement* stmt) {
  stmt->set_normal(normal_);
  stmt->set_raised(raised_);
  stmt->expr()->Accept(this);
}

// Expressions

void BindingResolver::Visit(Assignment* assign) {
  assign->left()->Accept(this);
  assign->right()->Accept(this);
}

void BindingResolver::Visit(BinaryOperation* binary) {
  binary->left()->Accept(this);
  binary->right()->Accept(this);
}

void BindingResolver::Visit(ConditionalExpression* cond) {
  cond->cond()->Accept(this);
  cond->left()->Accept(this);
  cond->right()->Accept(this);
}

void BindingResolver::Visit(UnaryOperation* unary) {
  unary->expr()->Accept(this);
}

void BindingResolver::Visit(PostfixExpression* postfix) {
  postfix->expr()->Accept(this);
}

void BindingResolver::Visit(StringLiteral* literal) {
  // contains no identifier
}

void BindingResolver::Visit(NumberLiteral* literal) {
  // contains no identifier
}

class FindBinding : public std::unary_function<const Binding*, bool> {
 public:
  FindBinding(Symbol sym) : target_(sym) { }
  result_type operator()(argument_type binding) const {
    return binding->name() == target_;
  }
 private:
  Symbol target_;
};

void BindingResolver::Visit(Identifier* ident) {
  // search Idenfier and make binding or set refer
  const Symbol target = Intern(ident->value());
  {
    // search in inner scope
    const Bindings::iterator it =
        std::find_if(inner_scope_->begin(),
                     inner_scope_->end(),
                     FindBinding(target));
    if (it != inner_scope_->end()) {
      // find binding in inner scope
      ident->set_binding_type(Binding::STACK);
      ident->set_refer(*it);
      return;
    }
  }
  {
    // search in outer scope
    const Bindings::iterator it =
        std::find_if(outer_scope_.begin(),
                     outer_scope_.end(),
                     FindBinding(target));
    if (it != outer_scope_.end()) {
      (*it)->ToHeap();
      ident->set_binding_type(Binding::HEAP);
      ident->set_refer(*it);
      // record that this heap variable is declared
      heap_->RecordDeclaredHeapBinding(*it);
      return;
    }
  }
  // global variables that is not declared
  // remeber this identifier will be treated as GLOBAL.ident access
  ident->set_binding_type(Binding::GLOBAL);
}

void BindingResolver::Visit(ThisLiteral* literal) {
  // contains no identifier
}

void BindingResolver::Visit(NullLiteral* lit) {
  // contains no identifier
}

void BindingResolver::Visit(TrueLiteral* lit) {
  // contains no identifier
}

void BindingResolver::Visit(FalseLiteral* lit) {
  // contains no identifier
}

void BindingResolver::Visit(RegExpLiteral* literal) {
  // contains no identifier
}

void BindingResolver::Visit(ArrayLiteral* literal) {
  // visit each elements
  for (MaybeExpressions::const_iterator it = literal->items().begin(),
       last = literal->items().end(); it != last; ++it) {
    if (const iv::core::Maybe<Expression> expr = *it) {
      expr.Address()->Accept(this);
    }
  }
}

void BindingResolver::Visit(ObjectLiteral* literal) {
  // visit each elements
  for (ObjectLiteral::Properties::const_iterator it = literal->properties().begin(),
       last = literal->properties().end(); it != last; ++it) {
    const ObjectLiteral::Property& prop = *it;
    if (std::get<0>(prop) == ObjectLiteral::DATA) {
      std::get<2>(prop)->Accept(this);
    }
  }
}

void BindingResolver::Visit(FunctionLiteral* literal) {
  Statement* prev_normal = normal_;
  Statement* prev_raised = raised_;
  normal_ = NULL;
  raised_ = NULL;
  // add inner_scope bindings to outer_scope
  const std::size_t previous_size = outer_scope_.size();
  if (inner_scope_) {
    // add previous inner scope bindings into outer scope
    outer_scope_.insert(outer_scope_.end(),
                        inner_scope_->begin(), inner_scope_->end());
  }
  Bindings* previous_inner_scope = inner_scope_;
  Bindings inner_scope;
  inner_scope_ = &inner_scope;

  // instantiate scope variables
  const Scope& scope = literal->scope();
  const FunctionLiteral::DeclType type = literal->type();
  for (Identifiers::const_iterator it = literal->params().begin(),
       last = literal->params().end(); it != last; ++it) {
    Identifier* ident = *it;
    Binding* binding = heap_->Instantiate(Intern(ident->value()));
    ident->set_refer(binding);
    inner_scope.push_back(binding);
  }
  if (type == FunctionLiteral::STATEMENT ||
      (type == FunctionLiteral::EXPRESSION && literal->name())) {
    Identifier* ident = literal->name().Address();
    Binding* binding = heap_->Instantiate(Intern(ident->value()));
    ident->set_refer(binding);
    inner_scope.push_back(binding);
  }
  for (Scope::Variables::const_iterator it = scope.variables().begin(),
       last = scope.variables().end(); it != last; ++it) {
    const Scope::Variable& var = *it;
    Identifier* ident = var.first;
    Binding* binding = heap_->Instantiate(Intern(ident->value()));
    ident->set_refer(binding);  // set refer binding to identifier
    inner_scope.push_back(binding);
    if (type == FunctionLiteral::GLOBAL) {
      heap_->RecordDeclaredHeapBinding(binding);
    }
  }
  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    Identifier* ident = (*it)->name().Address();
    Binding* binding = heap_->Instantiate(Intern(ident->value()));
    ident->set_refer(binding);  // set refer binding to identifier
    inner_scope.push_back(binding);
  }

  // realization
  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    (*it)->Accept(this);
  }

  // resolve binding of new function and mark continuations
  literal->set_raised(raised_);
  if (literal->body().empty()) {
    // no Statement, so next is normal
    literal->set_normal(normal_);
  } else {
    // first Statement
    literal->set_normal(literal->body().front());
  }
  MarkStatements(literal->body());

  // for completion phase
  if (heap_->completer()) {
    if (literal == heap_->completer()->GetTargetFunction()) {
      // this is target function
      heap_->completer()->GetTargetExpression()->Accept(this);
    }
  }

  // shrink outer scope to previous size (remove added inner scope bindings)
  outer_scope_.resize(previous_size);
  inner_scope_ = previous_inner_scope;
  normal_ = prev_normal;
  raised_ = prev_raised;
}

void BindingResolver::Visit(IdentifierAccess* prop) {
  // TODO(Constellation) handling arguments
  prop->target()->Accept(this);
}

void BindingResolver::Visit(IndexAccess* prop) {
  // TODO(Constellation) handling arguments
  prop->target()->Accept(this);
  prop->key()->Accept(this);
}

void BindingResolver::Visit(FunctionCall* call) {
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void BindingResolver::Visit(ConstructorCall* call) {
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

// Others

void BindingResolver::Visit(Declaration* dummy) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BINDING_RESOLVER_H_
