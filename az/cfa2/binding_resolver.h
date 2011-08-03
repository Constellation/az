// resolve which identifier is STACK or HEAP
#ifndef _AZ_CFA2_BINDING_RESOLVER_H_
#define _AZ_CFA2_BINDING_RESOLVER_H_
#include <iv/noncopyable.h>
#include <iv/maybe.h>
#include <az/ast_fwd.h>
#include <az/symbol.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/binding_resolver_fwd.h>
namespace az {
namespace cfa2 {

void BindingResolver::Visit(Block* block) {
  for (Statements::const_iterator it = block->body().begin(),
       last = block->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void BindingResolver::Visit(FunctionStatement* func) {
}

void BindingResolver::Visit(FunctionDeclaration* func) {
}

void BindingResolver::Visit(VariableStatement* var) {
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
}

void BindingResolver::Visit(ForInStatement* stmt) {
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
  stmt->context()->Accept(this);
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(LabelledStatement* stmt) {
  stmt->body()->Accept(this);
}

void BindingResolver::Visit(SwitchStatement* stmt) {
}

void BindingResolver::Visit(ThrowStatement* stmt) {
  stmt->expr()->Accept(this);
}

void BindingResolver::Visit(TryStatement* stmt) {
  stmt->body()->Accept(this);
  if (const iv::core::Maybe<Block> block = stmt->catch_block()) {
  } else {
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
      // record this variable is registered
      return;
    }
  }
  // global variables
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
  // add inner_scope bindings to outer_scope
  const std::size_t previous_size = outer_scope_.size();
  outer_scope_.insert(outer_scope_.end(),
                      inner_scope_->begin(), inner_scope_->end());
  Bindings inner_scope;
  inner_scope_ = &inner_scope;
  // instantiate scope variables

  // resolve binding of new function
  for (Statements::const_iterator it = literal->body().begin(),
       last = literal->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
  outer_scope_.resize(previous_size);
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