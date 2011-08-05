#ifndef _AZ_CFA2_HEAP_INITIALIZER_H_
#define _AZ_CFA2_HEAP_INITIALIZER_H_
#include <az/cfa2/heap_initializer_fwd.h>
#include <az/cfa2/builtins.h>
namespace az {
namespace cfa2 {

void HeapInitializer::Initialize(FunctionLiteral* global) {
}

// Statements

void HeapInitializer::Visit(Block* block) {
}

void HeapInitializer::Visit(FunctionStatement* func) {
  Visit(func->function());
}

void HeapInitializer::Visit(FunctionDeclaration* func) {
  Visit(func->function());
}

void HeapInitializer::Visit(VariableStatement* var) {
}

void HeapInitializer::Visit(EmptyStatement* stmt) {
}

void HeapInitializer::Visit(IfStatement* stmt) {
}

void HeapInitializer::Visit(DoWhileStatement* stmt) {
}

void HeapInitializer::Visit(WhileStatement* stmt) {
}

void HeapInitializer::Visit(ForStatement* stmt) {
}

void HeapInitializer::Visit(ForInStatement* stmt) {
}

void HeapInitializer::Visit(ContinueStatement* stmt) {
}

void HeapInitializer::Visit(BreakStatement* stmt) {
}

void HeapInitializer::Visit(ReturnStatement* stmt) {
}

void HeapInitializer::Visit(WithStatement* stmt) {
}

void HeapInitializer::Visit(LabelledStatement* stmt) {
}

void HeapInitializer::Visit(SwitchStatement* stmt) {
}

void HeapInitializer::Visit(ThrowStatement* stmt) {
}

void HeapInitializer::Visit(TryStatement* stmt) {
}

void HeapInitializer::Visit(DebuggerStatement* stmt) {
}

void HeapInitializer::Visit(ExpressionStatement* stmt) {
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
  AObject* regexp = heap_->GetFactory()->NewAObject();
  heap_->DeclObject(literal, regexp);
  REGEXP_CONSTRUCTOR(heap_, AVal(regexp), NULL, false);
}

void HeapInitializer::Visit(ArrayLiteral* literal) {
  // store AObject address to heap
  AObject* ary = heap_->GetFactory()->NewAObject();
  heap_->DeclObject(literal, ary);
  ARRAY_CONSTRUCTOR(heap_, AVal(ary), NULL, true);

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
    if (std::get<0>(prop) == ObjectLiteral::DATA) {
      std::get<2>(prop)->Accept(this);
    }
  }
}

void HeapInitializer::Visit(FunctionLiteral* literal) {
  AObject* obj = heap_->MakeFunction(literal);
  heap_->DeclObject(literal, obj);
  obj->AddProperty(
      Intern("prototype"),
      AProp(AVal(AVAL_NOBASE), A::W | A::C));

  if (iv::core::Maybe<Identifier> ident = literal->name()) {
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
  heap_->InitPending(obj);

  for (Statements::const_iterator it = literal->body().begin(),
       last = literal->body().end(); it != last; ++it) {
    (*it)->Accept(this);
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

void HeapInitializer::Visit(CaseClause* clause) {
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_DECL_INITIALIZER_H_
