#ifndef _AZ_CFA2_HEAP_INITIALIZER_H_
#define _AZ_CFA2_HEAP_INITIALIZER_H_
#include <algorithm>
#include <az/cfa2/heap_initializer_fwd.h>
#include <az/cfa2/builtins.h>
namespace az {
namespace cfa2 {

void HeapInitializer::Initialize(FunctionLiteral* global) {
  const Scope& scope = global->scope();

  for (Scope::Variables::const_iterator it = scope.variables().begin(),
       last = scope.variables().end(); it != last; ++it) {
    const Scope::Variable& var = *it;
    Binding* binding = var.first->refer();
    assert(binding);
    if (binding->type() == Binding::HEAP) {
      binding->set_value(AVal(AVAL_NOBASE));
    }
  }

  for (Statements::const_iterator it = global->body().begin(),
       last = global->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }

  // for completion phase
  if (heap_->completer()) {
    if (global == heap_->completer()->GetTargetFunction()) {
      // this is target function
      heap_->completer()->GetTargetExpression()->Accept(this);
    }
  }

  for (Scope::FunctionLiterals::const_iterator it = scope.GetFunctionLiteralsUnderThis().begin(),
       last = scope.GetFunctionLiteralsUnderThis().end(); it != last; ++it) {
    if (heap_->IsNotReachable(*it)) {
      // not reachable function
      Visit(*it);
    }
  }
}

// Statements

void HeapInitializer::Visit(Block* block) {
  for (Statements::const_iterator it = block->body().begin(),
       last = block->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void HeapInitializer::Visit(FunctionStatement* func) {
  Visit(func->function());
}

void HeapInitializer::Visit(FunctionDeclaration* func) {
  Visit(func->function());
}

void HeapInitializer::Visit(VariableStatement* var) {
  for (Declarations::const_iterator it = var->decls().begin(),
       last = var->decls().end(); it != last; ++it) {
    if (const iv::core::Maybe<Expression> expr = (*it)->expr()) {
      expr.Address()->Accept(this);
    }
  }
}

void HeapInitializer::Visit(EmptyStatement* stmt) {
  // nothing
}

void HeapInitializer::Visit(IfStatement* stmt) {
  stmt->cond()->Accept(this);
  stmt->then_statement()->Accept(this);
  if (const iv::core::Maybe<Statement> target = stmt->else_statement()) {
    target.Address()->Accept(this);
  }
}

void HeapInitializer::Visit(DoWhileStatement* stmt) {
  stmt->body()->Accept(this);
  stmt->cond()->Accept(this);
}

void HeapInitializer::Visit(WhileStatement* stmt) {
  stmt->cond()->Accept(this);
  stmt->body()->Accept(this);
}

void HeapInitializer::Visit(ForStatement* stmt) {
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

void HeapInitializer::Visit(ForInStatement* stmt) {
  stmt->each()->Accept(this);
  stmt->enumerable()->Accept(this);
  stmt->body()->Accept(this);
}

void HeapInitializer::Visit(ContinueStatement* stmt) {
  // nothing
}

void HeapInitializer::Visit(BreakStatement* stmt) {
  // nothing
}

void HeapInitializer::Visit(ReturnStatement* stmt) {
  if (const iv::core::Maybe<Expression> expr = stmt->expr()) {
    expr.Address()->Accept(this);
  }
}

void HeapInitializer::Visit(WithStatement* stmt) {
  // this is ambiguous, but use this
  stmt->context()->Accept(this);
  stmt->body()->Accept(this);
}

void HeapInitializer::Visit(LabelledStatement* stmt) {
  stmt->body()->Accept(this);
}

void HeapInitializer::Visit(SwitchStatement* stmt) {
  stmt->expr()->Accept(this);
  typedef SwitchStatement::CaseClauses CaseClauses;
  const CaseClauses& clauses = stmt->clauses();
  for (CaseClauses::const_iterator it = clauses.begin(),
       last = clauses.end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void HeapInitializer::Visit(CaseClause* clause) {
  if (const iv::core::Maybe<Expression> expr = clause->expr()) {
    expr.Address()->Accept(this);
  }
  for (Statements::const_iterator it = clause->body().begin(),
       last = clause->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void HeapInitializer::Visit(ThrowStatement* stmt) {
  stmt->expr()->Accept(this);
}

void HeapInitializer::Visit(TryStatement* stmt) {
  stmt->body()->Accept(this);
  if (const iv::core::Maybe<Block> block = stmt->catch_block()) {
    block.Address()->Accept(this);
  }
  if (const iv::core::Maybe<Block> block = stmt->finally_block()) {
    block.Address()->Accept(this);
  }
}

void HeapInitializer::Visit(DebuggerStatement* stmt) {
  // nothing
}

void HeapInitializer::Visit(ExpressionStatement* stmt) {
  stmt->expr()->Accept(this);
}

// Expressions

void HeapInitializer::Visit(Assignment* assign) {
  assign->left()->Accept(this);
  assign->right()->Accept(this);
}

void HeapInitializer::Visit(BinaryOperation* binary) {
  binary->left()->Accept(this);
  binary->right()->Accept(this);
}

void HeapInitializer::Visit(ConditionalExpression* cond) {
  cond->cond()->Accept(this);
  cond->left()->Accept(this);
  cond->right()->Accept(this);
}

void HeapInitializer::Visit(UnaryOperation* unary) {
  unary->expr()->Accept(this);
}

void HeapInitializer::Visit(PostfixExpression* postfix) {
  postfix->expr()->Accept(this);
}

void HeapInitializer::Visit(StringLiteral* literal) {
  // nothing
}

void HeapInitializer::Visit(NumberLiteral* literal) {
  // nothing
}

void HeapInitializer::Visit(Identifier* literal) {
  // nothing
}

void HeapInitializer::Visit(ThisLiteral* literal) {
  // nothing
}

void HeapInitializer::Visit(NullLiteral* lit) {
  // nothing
}

void HeapInitializer::Visit(TrueLiteral* lit) {
  // nothing
}

void HeapInitializer::Visit(FalseLiteral* lit) {
  // nothing
}

void HeapInitializer::Visit(RegExpLiteral* literal) {
  std::vector<AVal> args;
  AObject* regexp = heap_->GetFactory()->NewAObject();
  heap_->DeclObject(literal, regexp);
  REGEXP_CONSTRUCTOR(heap_, AVal(regexp), args, false);
}

void HeapInitializer::Visit(ArrayLiteral* literal) {
  // store AObject address to heap
  std::vector<AVal> args;
  AObject* ary = heap_->GetFactory()->NewAObject();
  heap_->DeclObject(literal, ary);
  ARRAY_CONSTRUCTOR(heap_, AVal(ary), args, true);

  // visit each elements
  for (MaybeExpressions::const_iterator it = literal->items().begin(),
       last = literal->items().end(); it != last; ++it) {
    if (const iv::core::Maybe<Expression> expr = *it) {
      expr.Address()->Accept(this);
    }
  }
}

void HeapInitializer::Visit(ObjectLiteral* literal) {
  AObject* obj = heap_->MakeObject();
  heap_->DeclObject(literal, obj);
  for (ObjectLiteral::Properties::const_iterator it = literal->properties().begin(),
       last = literal->properties().end(); it != last; ++it) {
    const ObjectLiteral::Property& prop = *it;
    Expression* expr = std::get<2>(prop);
    if (FunctionLiteral* literal = expr->AsFunctionLiteral()) {
      // specialized path
      heap_->DeclObjectLiteralMember(literal, obj);
    }
    expr->Accept(this);
  }
}

void HeapInitializer::Visit(FunctionLiteral* literal) {
  AObject* obj = heap_->MakeFunction(literal);
  heap_->DeclObject(literal, obj);
  AVal prototype = AVal(heap_->MakePrototype(obj));
  obj->AddProperty(Intern("prototype"), AProp(prototype, A::W | A::C));

  if (const iv::core::Maybe<Identifier> ident = literal->name()) {
    // function literal name has always binding
    Binding* binding = ident.Address()->refer();
    assert(binding);
    if (binding->type() == Binding::HEAP) {
      // this is heap variable, so initialize it (not stack)
      binding->set_value(AVal(obj));
    }
  }

  // parameter binding initialization
  for (Identifiers::const_iterator it = literal->params().begin(),
       last = literal->params().end(); it != last; ++it) {
    Binding* binding = (*it)->refer();
    assert(binding);
    if (binding->type() == Binding::HEAP) {
      binding->set_value(AVal(AVAL_NOBASE));
    }
  }

  // insert current function summary
  heap_->InitSummary(literal, obj);

  for (Statements::const_iterator it = literal->body().begin(),
       last = literal->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }

  // for completion phase
  if (heap_->completer()) {
    if (literal == heap_->completer()->GetTargetFunction()) {
      // this is target function
      heap_->completer()->GetTargetExpression()->Accept(this);
    }
  }

  const Scope& scope = literal->scope();

  for (Scope::FunctionLiterals::const_iterator it = scope.GetFunctionLiteralsUnderThis().begin(),
       last = scope.GetFunctionLiteralsUnderThis().end(); it != last; ++it) {
    if (heap_->IsNotReachable(*it)) {
      // not reachable function
      Visit(*it);
    }
  }
}

void HeapInitializer::Visit(IdentifierAccess* prop) {
  // TODO(Constellation) handling arguments
  prop->target()->Accept(this);
}

void HeapInitializer::Visit(IndexAccess* prop) {
  // TODO(Constellation) handling arguments
  prop->target()->Accept(this);
  prop->key()->Accept(this);
}

void HeapInitializer::Visit(FunctionCall* call) {
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void HeapInitializer::Visit(ConstructorCall* call) {
  heap_->DeclObject(call, heap_->GetFactory()->NewAObject());
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

// Others

void HeapInitializer::Visit(Declaration* dummy) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_DECL_INITIALIZER_H_
