#ifndef _AZ_CFA2_INTERPRETER_H_
#define _AZ_CFA2_INTERPRETER_H_
#include <deque>
#include <iv/debug.h>
#include <az/cfa2/interpreter_fwd.h>
#include <az/cfa2/heap_initializer.h>
#include <az/cfa2/frame.h>
namespace az {
namespace cfa2 {

class Work { };

void Interpreter::Run(FunctionLiteral* global) {
  Frame frame;
  frame_ = &frame;  // set current frame
  {
    // initialize heap
    //
    // initialize summaries and heap static objects declaration
    // static objects are bound to heap by AstNode address and
    // summaries are bound to heap by FunctionLiteral address
    HeapInitializer initializer(heap_);
    initializer.Initialize(global);
  }

  frame.SetThis(heap_->GetGlobal());
  const Scope& scope = global->scope();
  for (Scope::Variables::const_iterator it = scope.variables().begin(),
       last = scope.variables().end(); it != last; ++it) {
    const Scope::Variable& var = *it;
    Identifier* ident = var.first;
    assert(ident->refer());
    frame.Set(heap_, ident->refer(), AVal(AVAL_NOBASE));
  }

  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    const Symbol fn = Intern((*it)->name().Address()->value());
    Identifier* ident = (*it)->name().Address();
    assert(ident->refer());
    frame.Set(heap_, ident->refer(), AVal(heap_->GetDeclObject(*it)));
  }

  // then interpret global function
  Interpret(global);

  // summary update phase
  //
  // enumerate summaries and if this function is not summaried, so make this
  for (Summaries::const_iterator it = heap_->summaries().begin(),
       last = heap_->summaries().end(); it != last; ++it) {
    if (!it->second->IsExists()) {
      // not summarized yet
      const std::vector<AVal> vec(it->first->params().size(), AVal(AVAL_NOBASE));
      EvaluateFunction(it->first,
                       it->second->target(),
                       AVal(heap_->MakeObject()),
                       vec,
                       false);
    }
  }
}

// Statements

void Interpreter::Visit(Block* block) {
  // do nothing
}

void Interpreter::Visit(FunctionStatement* func) {
}

void Interpreter::Visit(FunctionDeclaration* func) {
  // do nothing
}

void Interpreter::Visit(VariableStatement* var) {
  for (Declarations::const_iterator it = var->decls().begin(),
       last = var->decls().end(); it != last; ++it) {
    Binding* binding = (*it)->name()->refer();
    if (const iv::core::Maybe<Expression> expr = (*it)->expr()) {
      expr.Address()->Accept(this);
    }
    if (std::get<1>(answer_)) {
      // add raised path to error queue
      errors_->push_back(std::make_pair(var->raised(), answer_));
      // surpress error
      std::get<1>(answer_) = false;
    }
    if (binding->type() == Binding::HEAP) {
      heap_->UpdateHeap(binding, AVal(AVAL_NUMBER));
    } else if (binding->type() == Binding::STACK) {
      const AVal val(frame_->Get(heap_, binding) | std::get<0>(answer_));
      frame_->Set(heap_, binding, val);
      if (heap_->IsDeclaredHeapBinding(binding)) {
        heap_->UpdateHeap(binding, val);
      }
    } else {
      // binding type is stack?
      // TODO(Constellation) fix it when global
    }
  }
}

void Interpreter::Visit(EmptyStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(IfStatement* stmt) {
  // else, then statement is evaluate by main loop
  stmt->cond()->Accept(this);
}

void Interpreter::Visit(DoWhileStatement* stmt) {
  // because cond expr is wrapped and chain to stmt->normal(),
  // so, do nothing
}

void Interpreter::Visit(WhileStatement* stmt) {
  stmt->cond()->Accept(this);
}

void Interpreter::Visit(ForStatement* stmt) {
  // because init, cond, next expr is wrapped and chain to stmt->normal(),
  // so, do nothing
}

void Interpreter::Visit(ForInStatement* stmt) {
  if (stmt->each()->AsVariableStatement()) {
    Visit(stmt->each()->AsVariableStatement());
  }
  stmt->enumerable()->Accept(this);
}

void Interpreter::Visit(ContinueStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(BreakStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(ReturnStatement* stmt) {
  if (const iv::core::Maybe<Expression> expr = stmt->expr()) {
    expr.Address()->Accept(this);
  } else {
    answer_ = Answer(AVal(AVAL_UNDEFINED), false, AVal(AVAL_NOBASE));
  }
}

void Interpreter::Visit(WithStatement* stmt) {
  stmt->context()->Accept(this);
}

void Interpreter::Visit(LabelledStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(SwitchStatement* stmt) {
  stmt->expr()->Accept(this);
}

void Interpreter::Visit(CaseClause* clause) {
  if (const iv::core::Maybe<Expression> expr = clause->expr()) {
    expr.Address()->Accept(this);
  }
}

void Interpreter::Visit(ThrowStatement* stmt) {
  stmt->expr()->Accept(this);
  AVal err(std::get<0>(answer_));  // normal result thrown
  // and if expr throw error, merge this
  if (std::get<1>(answer_)) {
    err.Join(std::get<2>(answer_));
  }
  answer_ = Answer(AVal(AVAL_NOBASE), true, err);
}


void Interpreter::Visit(TryStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(DebuggerStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(ExpressionStatement* stmt) {
  stmt->expr()->Accept(this);
}

// Expressions

void Interpreter::Visit(Assignment* assign) {
}

void Interpreter::Visit(BinaryOperation* binary) {
  using iv::core::Token;
  const Token::Type token = binary->op();
  switch (token) {
    case Token::TK_LOGICAL_AND:  // &&
    case Token::TK_LOGICAL_OR:  // ||
    case Token::TK_MUL:  // *
    case Token::TK_DIV:  // /
    case Token::TK_MOD:  // %
    case Token::TK_SUB:  // -
    case Token::TK_SHL:  // <<
    case Token::TK_SAR:  // >>
    case Token::TK_SHR:  // >>>
    case Token::TK_BIT_AND:  // &
    case Token::TK_BIT_XOR:  // ^
    case Token::TK_BIT_OR: {  // |
      // returns number
      binary->left()->Accept(this);
      const Answer la = answer_;
      binary->right()->Accept(this);
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (std::get<1>(la)) {
        error_found = true;
        err.Join(std::get<2>(la));
      }
      if (std::get<1>(answer_)) {
        error_found = true;
        err.Join(std::get<2>(answer_));
      }
      answer_ = Answer(AVal(AVAL_NUMBER), error_found, err);
      break;
    }

    case Token::TK_ADD: {  // +
      // add STRING + STRING is STRING
      binary->left()->Accept(this);
      const Answer la = answer_;
      binary->right()->Accept(this);
      const AVal res(std::get<0>(answer_) + std::get<0>(la));
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (std::get<1>(la)) {
        error_found = true;
        err.Join(std::get<2>(la));
      }
      if (std::get<1>(answer_)) {
        error_found = true;
        err.Join(std::get<2>(answer_));
      }
      answer_ = Answer(res, error_found, err);
      break;
    }

    case Token::TK_LT:  // <
    case Token::TK_GT:  // >
    case Token::TK_LTE:  // <=
    case Token::TK_GTE:  // >=
    case Token::TK_INSTANCEOF:  // instanceof
    case Token::TK_IN:  // in
    case Token::TK_EQ:  // ==
    case Token::TK_NE:  // !=
    case Token::TK_EQ_STRICT:  // ===
    case Token::TK_NE_STRICT: {  // !==
      // returns bool
      binary->left()->Accept(this);
      const Answer la = answer_;
      binary->right()->Accept(this);
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (std::get<1>(la)) {
        error_found = true;
        err.Join(std::get<2>(la));
      }
      if (std::get<1>(answer_)) {
        error_found = true;
        err.Join(std::get<2>(answer_));
      }
      answer_ = Answer(AVal(AVAL_BOOL), error_found, err);
      break;
    }

    case Token::TK_COMMA: {  // ,
      binary->left()->Accept(this);
      const Answer la = answer_;
      binary->right()->Accept(this);
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (std::get<1>(la)) {
        error_found = true;
        err.Join(std::get<2>(la));
      }
      if (std::get<1>(answer_)) {
        error_found = true;
        err.Join(std::get<2>(answer_));
      }
      answer_ = Answer(std::get<0>(answer_), error_found, err);
      break;
    }

    default: {
      UNREACHABLE();
      break;
    }
  }
}

void Interpreter::Visit(ConditionalExpression* cond) {
  cond->cond()->Accept(this);
  Answer ca = answer_;  // cond answer
  cond->left()->Accept(this);
  Answer la = answer_;  // left answer
  cond->right()->Accept(this);
  Answer ra = answer_;  // right answer
  // aggregate error and result values
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  if (std::get<1>(ca)) {
    error_found = true;
    err.Join(std::get<2>(ca));
  }
  if (std::get<1>(la)) {
    error_found = true;
    err.Join(std::get<2>(la));
  }
  if (std::get<1>(ra)) {
    error_found = true;
    err.Join(std::get<2>(ra));
  }
  answer_ = Answer(
      std::get<0>(la) | std::get<0>(ra),
      error_found,
      err);
}

void Interpreter::Visit(UnaryOperation* unary) {
  // from iv / lv5 teleporter
  using iv::core::Token;
  switch (unary->op()) {
    case Token::TK_DELETE: {
      // if no error occurred, this always returns BOOL / UNDEFINED
      unary->expr()->Accept(this);
      answer_ = Answer(
          AVal(AVAL_BOOL) | AVal(AVAL_UNDEFINED),
          std::get<1>(answer_),
          std::get<2>(answer_));
      return;
    }

    case Token::TK_VOID: {
      // if no error occurred, this always returns UNDEFINED
      unary->expr()->Accept(this);
      answer_ = Answer(
          AVal(AVAL_UNDEFINED),
          std::get<1>(answer_),
          std::get<2>(answer_));
      return;
    }

    case Token::TK_TYPEOF: {
      // if no error occurred, this always returns STRING
      unary->expr()->Accept(this);
      answer_ = Answer(
          AVal(AVAL_STRING),
          std::get<1>(answer_),
          std::get<2>(answer_));
      return;
    }

    case Token::TK_INC:
    case Token::TK_DEC:
    case Token::TK_ADD:
    case Token::TK_SUB:
    case Token::TK_BIT_NOT: {
      // if no error occurred, this always returns NUMBER
      unary->expr()->Accept(this);
      answer_ = Answer(
          AVal(AVAL_NUMBER),
          std::get<1>(answer_),
          std::get<2>(answer_));
      return;
    }

    case Token::TK_NOT: {
      unary->expr()->Accept(this);
      answer_ = Answer(
          AVal(AVAL_BOOL),
          std::get<1>(answer_),
          std::get<2>(answer_));
      return;
    }

    default:
      UNREACHABLE();
  }
}

void Interpreter::Visit(PostfixExpression* postfix) {
  postfix->expr()->Accept(this);
  answer_ = Answer(
      AVal(AVAL_NUMBER),
      std::get<1>(answer_),
      std::get<2>(answer_));
}

void Interpreter::Visit(StringLiteral* literal) {
  answer_ = Answer(AVal(literal->value()), false, AVal(AVAL_NOBASE));
}

void Interpreter::Visit(NumberLiteral* literal) {
  answer_ = Answer(AVal(AVAL_NUMBER), false, AVal(AVAL_NOBASE));
}

void Interpreter::Visit(Identifier* ident) {
  // lookup!
  Binding* binding = ident->refer();
  if (binding) {
    if (ident->refer()->type() == Binding::HEAP) {
      // this is heap variable, so lookup from heap
      answer_ = Answer(
          binding->value(),
          false, AVal(AVAL_NOBASE));
    } else {
      // this is stack variable, so lookup from frame
      answer_ = Answer(
          frame_->Get(heap_, binding),
          false, AVal(AVAL_NOBASE));
    }
  } else {
    // not found => global lookup
    // TODO(Constellation) implement it
  }
}

void Interpreter::Visit(ThisLiteral* literal) {
  answer_ = Answer(frame_->GetThis(), false, AVal(AVAL_NOBASE));
}

void Interpreter::Visit(NullLiteral* lit) {
  answer_ = Answer(AVal(AVAL_NULL), false, AVal(AVAL_NOBASE));
}

void Interpreter::Visit(TrueLiteral* lit) {
  answer_ = Answer(AVal(AVAL_BOOL), false, AVal(AVAL_NOBASE));
}

void Interpreter::Visit(FalseLiteral* lit) {
  answer_ = Answer(AVal(AVAL_BOOL), false, AVal(AVAL_NOBASE));
}

void Interpreter::Visit(RegExpLiteral* literal) {
  answer_ = Answer(AVal(heap_->GetDeclObject(literal)), false, AVal(AVAL_NOBASE));
}

void Interpreter::Visit(ArrayLiteral* literal) {
  // visit each elements
  AObject* ary = heap_->GetDeclObject(literal);
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  uint32_t index = 0;
  for (MaybeExpressions::const_iterator it = literal->items().begin(),
       last = literal->items().end(); it != last; ++it, ++index) {
    if (const iv::core::Maybe<Expression> expr = *it) {
      expr.Address()->Accept(this);
      ary->UpdateProperty(heap_, Intern(index), std::get<0>(answer_));
      if (std::get<1>(answer_)) {
        // error found
        error_found = true;
        err.Join(std::get<2>(answer_));
      }
    }
  }
  answer_ = Answer(AVal(ary), error_found, err);
}

void Interpreter::Visit(ObjectLiteral* literal) {
  // visit each elements
  AObject* obj = heap_->GetDeclObject(literal);
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  for (ObjectLiteral::Properties::const_iterator it = literal->properties().begin(),
       last = literal->properties().end(); it != last; ++it) {
    const ObjectLiteral::Property& prop = *it;
    if (std::get<0>(prop) == ObjectLiteral::DATA) {
      std::get<2>(prop)->Accept(this);
      Identifier* ident = std::get<1>(prop);
      obj->UpdateProperty(heap_, Intern(ident->value()), std::get<0>(answer_));
      if (std::get<1>(answer_)) {
        // error found
        error_found = true;
        err.Join(std::get<2>(answer_));
      }
    }
  }
  answer_ = Answer(AVal(obj), error_found, err);
}

void Interpreter::Visit(FunctionLiteral* literal) {
  answer_ = Answer(AVal(heap_->GetDeclObject(literal)), false, AVal(AVAL_NOBASE));
}

void Interpreter::Interpret(FunctionLiteral* literal) {
  // interpret function
  // BindingResolver already marks normal / raised continuation
  // so this function use this and walking flow and evaluate this.
  AVal result(AVAL_NOBASE);
  AVal error(AVAL_NOBASE);
  bool error_found;
  Tasks tasks;
  Tasks* previous_tasks = tasks_;
  tasks_ = &tasks;  // set

  Errors errors;
  Errors* previous_errors = errors_;
  errors_ = &errors;  // set

  tasks_->push_back(literal->normal());
  while (true) {
    if (tasks_->empty()) {
      // all task is done!
      break;
    }
    Statement* const task = tasks_->back();
    tasks_->pop_back();
    if (!task) {
      continue;  // next statement
    }
    // patching phase
    if (task->AsReturnStatement()) {
      task->Accept(this);
      result.Join(std::get<0>(answer_));
    } else {
      task->Accept(this);
    }
    tasks_->push_back(task->normal());
    // check answer value and determine evaluate raised path or not
    if (std::get<1>(answer_)) {
      error_found = true;
      // raised path is catch / finally / NULL
      // if catch   ... TryStatement,
      //    finally ... Block
      if (task->raised()) {  // if raised path is found
        if (TryStatement* raised = task->raised()->AsTryStatement()) {
          error_found = false;
          assert(raised->catch_name() && raised->catch_block());
          Binding* binding = raised->catch_name().Address()->refer();
          if (frame_->IsDefined(heap_, binding)) {
            frame_->Set(
                heap_,
                binding,
                std::get<2>(answer_) | frame_->Get(heap_, binding));
          } else {
            frame_->Set(heap_, binding, std::get<2>(answer_));
          }
          tasks_->push_back(raised->catch_block().Address());
        } else {
          tasks_->push_back(task->raised());
        }
      } else {
        error.Join(std::get<2>(answer_));
      }
    }


    // treat errors
    // TODO(Constellation) clean up code
    for (Errors::const_iterator it = errors_->begin(),
         last = errors_->end(); it != last; ++it) {
      error_found = true;
      if (it->first) {
        if (TryStatement* raised = task->AsTryStatement()) {
          error_found = false;
          assert(raised->catch_name() && raised->catch_block());
          Binding* binding = raised->catch_name().Address()->refer();
          if (frame_->IsDefined(heap_, binding)) {
            frame_->Set(
                heap_,
                binding,
                std::get<2>(answer_) | frame_->Get(heap_, binding));
          } else {
            frame_->Set(heap_, binding, std::get<2>(answer_));
          }
        }
        tasks_->push_back(task->raised());
      } else {
        error.Join(std::get<2>(it->second));
      }
    }
    errors_->clear();
  }
  answer_ = Answer(result, error_found, error);
  tasks_ = previous_tasks;
  errors_ = previous_errors;
}

void Interpreter::Visit(IdentifierAccess* prop) {
}

void Interpreter::Visit(IndexAccess* prop) {
}

void Interpreter::Visit(FunctionCall* call) {
}

void Interpreter::Visit(ConstructorCall* call) {
}

// Others

void Interpreter::Visit(Declaration* dummy) {
}

Answer Interpreter::EvaluateFunction(FunctionLiteral* literal,
                                     AObject* function,
                                     const AVal& this_binding,
                                     const std::vector<AVal>& args,
                                     bool IsConstructorCalled) {
  if (function->builtin()) {
    return function->builtin()(heap_, this_binding, args, IsConstructorCalled);
  }

  heap_->CountUpCall();
  Answer result;
  if (heap_->FindSummary(function, this_binding, args, &result)) {
    // summary found. so, use this result
    return result;
  }
  heap_->CountUpDepth();

  // interpret function
  //
  Frame* previous = frame_;
  Frame frame;
  frame_ = &frame;

  frame_->SetThis(this_binding);
  const FunctionLiteral::DeclType type = literal->type();
  if (type == FunctionLiteral::STATEMENT ||
      (type == FunctionLiteral::EXPRESSION && literal->name())) {
    // in scope, so set to the frame
    Identifier* name = literal->name().Address();
    frame_->Set(heap_, name->refer(), AVal(heap_->GetDeclObject(literal)));
  }

  // parameter binding initialization
  std::size_t index = 0;
  const std::size_t args_size = args.size();
  for (Identifiers::const_iterator it = literal->params().begin(),
       last = literal->params().end(); it != last; ++it, ++index) {
    Binding* binding = (*it)->refer();
    assert(binding);
    const AVal target(args_size > index ? args[index] : AVal(AVAL_UNDEFINED));
    if (binding->type() == Binding::HEAP) {
      heap_->UpdateHeap(binding, target);
    }
    frame_->Set(heap_, binding, target);
  }

  if (index < args_size) {
    AVal result(AVAL_NOBASE);
    for (;index < args_size; ++index) {
      result.Join(args[index]);
    }
    frame_->SetRest(result);
  }

  const Scope& scope = literal->scope();
  for (Scope::Variables::const_iterator it = scope.variables().begin(),
       last = scope.variables().end(); it != last; ++it) {
    const Scope::Variable& var = *it;
    Identifier* ident = var.first;
    assert(ident->refer());
    frame_->Set(heap_, ident->refer(), AVal(AVAL_NOBASE));
  }

  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    const Symbol fn = Intern((*it)->name().Address()->value());
    Identifier* ident = (*it)->name().Address();
    assert(ident->refer());
    frame_->Set(heap_, ident->refer(), AVal(heap_->GetDeclObject(*it)));
  }

  // then, interpret
  Interpret(literal);

  heap_->AddSummary(function, this_binding, args, answer_);

  frame_ = previous;

  return result;
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_INTERPRETER_H_
