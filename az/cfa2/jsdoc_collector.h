// collect @constructor / @interface jsdoc information
#ifndef AZ_CFA2_JSDOC_COLLECTOR_H_
#define AZ_CFA2_JSDOC_COLLECTOR_H_
#include <algorithm>
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <az/cfa2/jsdoc_collector_fwd.h>
namespace az {
namespace cfa2 {

void JSDocCollector::Collect(FunctionLiteral* global) {
  Visit(global);
}

void JSDocCollector::Visit(Block* block) {
  std::for_each(block->body().begin(),
                block->body().end(),
                Acceptor<JSDocCollector>(this));
}

void JSDocCollector::Visit(FunctionStatement* func) {
  Visit(func->function());
}

void JSDocCollector::Visit(FunctionDeclaration* func) {
  Visit(func->function());
}

void JSDocCollector::Visit(VariableStatement* var) {
  for (Declarations::const_iterator it = var->decls().begin(),
       last = var->decls().end(); it != last; ++it) {
    if (const iv::core::Maybe<Expression> expr = (*it)->expr()) {
      expr.Address()->Accept(this);
    }
  }
}

void JSDocCollector::Visit(EmptyStatement* stmt) {
}

void JSDocCollector::Visit(IfStatement* stmt) {
  stmt->cond()->Accept(this);
  stmt->then_statement()->Accept(this);
  if (const iv::core::Maybe<Statement> target = stmt->else_statement()) {
    target.Address()->Accept(this);
  }
}

void JSDocCollector::Visit(DoWhileStatement* stmt) {
  stmt->body()->Accept(this);
  stmt->cond()->Accept(this);
}

void JSDocCollector::Visit(WhileStatement* stmt) {
  stmt->cond()->Accept(this);
  stmt->body()->Accept(this);
}

void JSDocCollector::Visit(ForStatement* stmt) {
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

void JSDocCollector::Visit(ForInStatement* stmt) {
  stmt->each()->Accept(this);
  stmt->enumerable()->Accept(this);
  stmt->body()->Accept(this);
}

void JSDocCollector::Visit(ContinueStatement* stmt) {
}

void JSDocCollector::Visit(BreakStatement* stmt) {
}

void JSDocCollector::Visit(ReturnStatement* stmt) {
  if (const iv::core::Maybe<Expression> expr = stmt->expr()) {
    expr.Address()->Accept(this);
  }
}

void JSDocCollector::Visit(WithStatement* stmt) {
  stmt->context()->Accept(this);
  stmt->body()->Accept(this);
}

void JSDocCollector::Visit(LabelledStatement* stmt) {
  stmt->body()->Accept(this);
}

void JSDocCollector::Visit(SwitchStatement* stmt) {
  typedef SwitchStatement::CaseClauses CaseClauses;
  const CaseClauses& clauses = stmt->clauses();
  stmt->expr()->Accept(this);
  for (CaseClauses::const_iterator it = clauses.begin(),
       last = clauses.end(); it != last; ++it) {
    Visit(*it);
  }
}

void JSDocCollector::Visit(CaseClause* clause) {
}

void JSDocCollector::Visit(ThrowStatement* stmt) {
  stmt->expr()->Accept(this);
}

void JSDocCollector::Visit(TryStatement* stmt) {
  stmt->body()->Accept(this);
  if (const iv::core::Maybe<Block> block = stmt->catch_block()) {
    Visit(block.Address());
  }
  if (const iv::core::Maybe<Block> block = stmt->finally_block()) {
    Visit(block.Address());
  }
}

void JSDocCollector::Visit(DebuggerStatement* stmt) {
}

void JSDocCollector::Visit(ExpressionStatement* stmt) {
  stmt->expr()->Accept(this);
}

// Expressions

void JSDocCollector::Visit(Assignment* assign) {
  assign->left()->Accept(this);
  assign->right()->Accept(this);
  if (FunctionLiteral* literal = assign->right()->AsFunctionLiteral()) {
    if (std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(assign)) {
      // adding jsdoc to function
      heap_->Tag(literal, info);
      if (info->GetTag(jsdoc::Token::TK_CONSTRUCTOR) ||
          info->GetTag(jsdoc::Token::TK_INTERFACE)) {
        DebugLog("@constructor / @interface found");
        heap_->registry()->RegisterAssignedType(assign->left(), literal);
      }
    }
  }
}

void JSDocCollector::Visit(BinaryOperation* binary) {
  binary->left()->Accept(this);
  binary->right()->Accept(this);
}

void JSDocCollector::Visit(ConditionalExpression* cond) {
  cond->cond()->Accept(this);
  cond->left()->Accept(this);
  cond->right()->Accept(this);
}

void JSDocCollector::Visit(UnaryOperation* unary) {
  unary->expr()->Accept(this);
}

void JSDocCollector::Visit(PostfixExpression* postfix) {
  postfix->expr()->Accept(this);
}

void JSDocCollector::Visit(StringLiteral* literal) {
}

void JSDocCollector::Visit(NumberLiteral* literal) {
}

void JSDocCollector::Visit(Identifier* ident) {
}

void JSDocCollector::Visit(ThisLiteral* literal) {
}

void JSDocCollector::Visit(NullLiteral* lit) {
}

void JSDocCollector::Visit(TrueLiteral* lit) {
}

void JSDocCollector::Visit(FalseLiteral* lit) {
}

void JSDocCollector::Visit(RegExpLiteral* literal) {
}

void JSDocCollector::Visit(ArrayLiteral* literal) {
  for (MaybeExpressions::const_iterator it = literal->items().begin(),
       last = literal->items().end(); it != last; ++it) {
    if (const iv::core::Maybe<Expression> expr = *it) {
      expr.Address()->Accept(this);
    }
  }
}

void JSDocCollector::Visit(ObjectLiteral* literal) {
  for (ObjectLiteral::Properties::const_iterator it = literal->properties().begin(),
       last = literal->properties().end(); it != last; ++it) {
    const ObjectLiteral::Property& prop = *it;
    std::get<2>(prop)->Accept(this);
  }
}

void JSDocCollector::Visit(FunctionLiteral* literal) {
  std::for_each(literal->body().begin(),
                literal->body().end(),
                Acceptor<JSDocCollector>(this));
  // tagging jsdoc information
  if (std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(literal)) {
    if (info->GetTag(jsdoc::Token::TK_CONSTRUCTOR) ||
        info->GetTag(jsdoc::Token::TK_INTERFACE)) {
      heap_->registry()->RegisterNamedType(literal);
    }
  }
}

void JSDocCollector::Visit(IdentifierAccess* prop) {
  prop->target()->Accept(this);
}

void JSDocCollector::Visit(IndexAccess* prop) {
  prop->target()->Accept(this);
  prop->key()->Accept(this);
}

void JSDocCollector::Visit(FunctionCall* call) {
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

void JSDocCollector::Visit(ConstructorCall* call) {
  call->target()->Accept(this);
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it) {
    (*it)->Accept(this);
  }
}

// Others

void JSDocCollector::Visit(Declaration* dummy) {
}

} }  // namespace az::cfa2
#endif  // AZ_CFA2_JSDOC_COLLECTOR_H_
