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

void BindingResolver::Resolve(FunctionLiteral* global) {
  Visit(global);
}

void BindingResolver::Visit(Block* block) {
  for (Statements::const_iterator it = block->body().begin(),
       last = block->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void BindingResolver::Visit(FunctionStatement* func) {
  func->function()->Accept(this);
}

void BindingResolver::Visit(FunctionDeclaration* func) {
  // nothing
}

void BindingResolver::Visit(VariableStatement* var) {
  for (Declarations::const_iterator it = var->decls().begin(),
       last = var->decls().end(); it != last; ++it) {
    if (const iv::core::Maybe<Expression> expr = (*it)->expr()) {
      expr.Address()->Accept(this);
    }
  }
}

void BindingResolver::Visit(EmptyStatement* stmt) {
  // not have Identifier to value
}

void BindingResolver::Visit(IfStatement* stmt) {
  stmt->cond()->Accept(this);
  stmt->then_statement()->Accept(this);
  if (const iv::core::Maybe<Statement> target = stmt->else_statement()) {
    target.Address()->Accept(this);
  }
}

void BindingResolver::Visit(DoWhileStatement* stmt) {
  stmt->body()->Accept(this);
  stmt->cond()->Accept(this);
}

void BindingResolver::Visit(WhileStatement* stmt) {
  stmt->cond()->Accept(this);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(ForStatement* stmt) {
  if (const iv::core::Maybe<Statement> init = stmt->init()) {
    init.Address()->Accept(this);
  }
  if (const iv::core::Maybe<Expression> cond = stmt->cond()) {
    cond.Address()->Accept(this);
  }
  stmt->body()->Accept(this);
  if (const iv::core::Maybe<Expression> next = stmt->next()) {
    next.Address()->Accept(this);
  }
}

void BindingResolver::Visit(ForInStatement* stmt) {
  stmt->each()->Accept(this);
  stmt->enumerable()->Accept(this);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(ContinueStatement* stmt) {
  // not have Identifier to value
}

void BindingResolver::Visit(BreakStatement* stmt) {
  // not have Identifier to value
}

void BindingResolver::Visit(ReturnStatement* stmt) {
  if (const iv::core::Maybe<Expression> expr = stmt->expr()) {
    expr.Address()->Accept(this);
  }
}

void BindingResolver::Visit(WithStatement* stmt) {
  // this is ambiguous, but use this
  stmt->context()->Accept(this);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(LabelledStatement* stmt) {
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(SwitchStatement* stmt) {
  stmt->expr()->Accept(this);
  typedef SwitchStatement::CaseClauses CaseClauses;
  const CaseClauses& clauses = stmt->clauses();
  for (CaseClauses::const_iterator it = clauses.begin(),
       last = clauses.end(); it != last; ++it) {
    CaseClause* const clause = *it;
    if (const iv::core::Maybe<Expression> expr = clause->expr()) {
      expr.Address()->Accept(this);
    }
    for (Statements::const_iterator it = clause->body().begin(),
         last = clause->body().end(); it != last; ++it) {
      (*it)->Accept(this);
    }
  }
}

void BindingResolver::Visit(ThrowStatement* stmt) {
  stmt->expr()->Accept(this);
}

void BindingResolver::Visit(TryStatement* stmt) {
  stmt->body()->Accept(this);
  if (const iv::core::Maybe<Block> block = stmt->catch_block()) {
    const Symbol name = Intern(stmt->catch_name().Address()->value());
    inner_scope_->push_back(heap_->Instantiate(name));
    block.Address()->Accept(this);
    inner_scope_->pop_back();
  }
  if (const iv::core::Maybe<Block> block = stmt->finally_block()) {
    block.Address()->Accept(this);
  }
}

void BindingResolver::Visit(DebuggerStatement* stmt) {
  // not have Identifier to value
}

void BindingResolver::Visit(ExpressionStatement* stmt) {
  stmt->expr()->Accept(this);
}

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
      std::cout << "STACK" << std::endl;
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
      std::cout << "HEAP" << std::endl;
      return;
    }
  }
  // global variables that is not declared
  // remeber this identifier will be treated as GLOBAL.ident access
  ident->set_binding_type(Binding::GLOBAL);
  std::cout << "GLOBAL" << std::endl;
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
    inner_scope.push_back(heap_->Instantiate(Intern((*it)->value())));
  }
  if (type == FunctionLiteral::STATEMENT ||
      (type == FunctionLiteral::EXPRESSION && literal->name())) {
    const Symbol name = Intern(literal->name().Address()->value());
    inner_scope.push_back(heap_->Instantiate(name));
  }
  for (Scope::Variables::const_iterator it = scope.variables().begin(),
       last = scope.variables().end(); it != last; ++it) {
    const Scope::Variable& var = *it;
    const Symbol dn = Intern(var.first->value());
    Binding* binding = heap_->Instantiate(dn);
    inner_scope.push_back(binding);
    if (type == FunctionLiteral::GLOBAL) {
      heap_->RecordDeclaredHeapBinding(binding);
    }
  }
  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    const Symbol fn = Intern((*it)->name().Address()->value());
    inner_scope.push_back(heap_->Instantiate(fn));
  }

  // realization
  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    (*it)->Accept(this);
  }

  // resolve binding of new function
  for (Statements::const_iterator it = literal->body().begin(),
       last = literal->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }

  // fix up


  // shrink outer scope to previous size (remove added inner scope bindings)
  outer_scope_.resize(previous_size);
  inner_scope_ = previous_inner_scope;
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

void BindingResolver::Visit(Declaration* dummy) {
}

void BindingResolver::Visit(CaseClause* dummy) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_BINDING_RESOLVER_H_
