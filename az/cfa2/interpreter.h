#ifndef AZ_CFA2_INTERPRETER_H_
#define AZ_CFA2_INTERPRETER_H_
#include <iv/debug.h>
#include <az/cfa2/interpreter_fwd.h>
#include <az/cfa2/aobject.h>
#include <az/cfa2/aval.h>
#include <az/cfa2/heap_initializer.h>
#include <az/cfa2/frame.h>
#include <az/cfa2/result.h>
#include <az/cfa2/type_registry.h>
#include <az/cfa2/param_type_matcher.h>
#include <az/cfa2/method_target_guard.h>
namespace az {
namespace cfa2 {

void Interpreter::Run(FunctionLiteral* global) {
  Frame frame(this);
  frame.SetThis(heap_->GetGlobal());
  const Scope& scope = global->scope();
  for (Scope::Variables::const_iterator it = scope.variables().begin(),
       last = scope.variables().end(); it != last; ++it) {
    const Scope::Variable& var = *it;
    Identifier* ident = var.first;
    Binding* binding = ident->refer();
    assert(binding);
    frame.Set(heap_, binding, AVal(AVAL_NOBASE));
  }

  for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
       last = scope.function_declarations().end(); it != last; ++it) {
    Identifier* ident = (*it)->name().Address();
    Binding* binding = ident->refer();
    assert(binding);
    const AVal val(heap_->GetDeclObject(*it));
    frame.Set(heap_, binding, val);
    heap_->GetGlobal().UpdateProperty(heap_, ident->symbol(), val);
  }

  // then interpret global function
  Interpret(global);

  assert(CurrentFrame() == &frame);

  // summary update phase
  //
  // enumerate summaries and if this function is not summaried, so make this
  for (std::vector<Summary*>::const_iterator it = heap_->ordered_summaries().begin(),
       last = heap_->ordered_summaries().end(); it != last; ++it) {
    Summary* current = *it;
    if (!current->IsExists()) {
      // not summarized yet
      FunctionLiteral* function = current->function();
      AObject* target = current->target();
      const std::vector<AVal> vec(function->params().size(), AVal(AVAL_NOBASE));

      std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(function);
      if (info) {
        if (info->GetTag(jsdoc::Token::TK_CONSTRUCTOR) ||
            info->GetTag(jsdoc::Token::TK_INTERFACE)) {
          // if jsdoc found and @constructor or @interface is specified,
          // call this function as constructor
          AObject* this_binding = heap_->MakeObject();
          this_binding->UpdatePrototype(heap_,
                                        target->GetProperty(Intern("prototype")));
          EvaluateFunction(target, AVal(this_binding), vec, true);
          continue;
        }
        if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_THIS)) {
          // @this is specified
          assert(tag->type());
          tag->type()->Accept(this);
          EvaluateFunction(target, result_.result(), vec, false);
          continue;
        }
      }

      if (AObject* obj = heap_->GetLiteralMemberBase(function)) {
        // such as,
        //   var obj ={
        //     test: function() { }
        //   };
        EvaluateFunction(target, AVal(obj), vec, false);
      } else if (Expression* expr = heap_->IsPrototypeMethod(function)) {
        // such as,
        //   Test.prototype.getValue = function() { ... }
        AVal constructor = heap_->GetMethodTarget(expr);
        if (constructor == AVal(AVAL_NOBASE)) {
          expr->Accept(this);
          constructor = result_.result();
        }
        if (constructor == AVal(AVAL_NOBASE)) {
          EvaluateFunction(target, AVal(heap_->MakeObject()), vec, false);
        } else {
          constructor.Construct(heap_,
                                this, heap_->MakeObject(), vec, &result_);
          EvaluateFunction(target, result_.result(), vec, false);
        }
      } else {
        EvaluateFunction(target, AVal(heap_->MakeObject()), vec, false);
      }
    }
  }

  // for completion, interpret it once more
  if (heap_->completer() && heap_->completer()->GetTargetFunction()) {
    if (heap_->completer()->GetTargetFunction() == global) {
      // global evaluation
      Interpret(global);
    } else {
      Completer* completer = heap_->completer();
      FunctionLiteral* compl_literal = completer->GetTargetFunction();
      Summary* summary =
          heap_->GetSummaryByFunction(compl_literal);
      const Summary::Entry& entry = summary->type();
      EvaluateFunction(summary->target(),
                       entry.this_binding(), entry.args(), false);
    }
  }
}

// Statements

void Interpreter::Visit(Block* block) {
  // do nothing
}

void Interpreter::Visit(FunctionStatement* func) {
  // do nothing
}

void Interpreter::Visit(FunctionDeclaration* func) {
  // do nothing
}

void Interpreter::Visit(VariableStatement* var) {
  AVal error(AVAL_NOBASE);
  bool error_found = false;
  for (Declarations::const_iterator it = var->decls().begin(),
       last = var->decls().end(); it != last; ++it) {
    result_.Reset();
    Identifier* ident = (*it)->name();
    Binding* binding = ident->refer();
    assert(binding);

    // this Identifier may have JSDoc
    bool jsdoc_type_is_found = false;
    AVal from_jsdoc(AVAL_NOBASE);
    if (std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(ident)) {
      if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_TYPE)) {
        DebugLog("@type found");
        jsdoc_type_is_found = true;
        assert(tag->type());
        tag->type()->Accept(this);
        from_jsdoc = result_.result();
      }
    }

    if (const iv::core::Maybe<Expression> expr = (*it)->expr()) {
      expr.Address()->Accept(this);
    }

    if (jsdoc_type_is_found) {
      result_.MergeResult(from_jsdoc);
    }
    if (result_.HasException()) {
      // collecting errors
      error_found = true;
      error |= result_.exception();
    }
    if (binding->type() == Binding::HEAP) {
      heap_->UpdateHeap(binding, result_.result());
    } else if (binding->type() == Binding::STACK) {
      const AVal val(CurrentFrame()->Get(heap_, binding) | result_.result());
      CurrentFrame()->Set(heap_, binding, val);
      if (heap_->IsDeclaredHeapBinding(binding)) {
        // Global variable
        heap_->UpdateHeap(binding, val);
        heap_->GetGlobal().UpdateProperty(heap_, ident->symbol(), val);
      }
    }
  }
  result_ = Result(AVal(AVAL_NOBASE), error, error_found);
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
  Identifier* ident = NULL;
  if (stmt->each()->AsVariableStatement()) {
    VariableStatement* var = stmt->each()->AsVariableStatement();
    if (!var->decls().empty()) {
      ident = var->decls().front()->name();
    }
    Visit(var);
  } else {
    ExpressionStatement* ex = stmt->each()->AsExpressionStatement();
    ident = ex->expr()->AsIdentifier();
  }
  stmt->enumerable()->Accept(this);

  // too heavy...
  // so turn off this option
  if (heap_->for_in_handling()) {
    const AVal res = result_.result();
    if (ident) {
      if (Binding* binding = ident->refer()) {
        if (binding->type() == Binding::STACK) {
          Result prev = result_;
          // limit different name symbol only
          std::unordered_set<Symbol> already;
          // too heavy, so only one statement allowed...
          // very heuristic...
          if (Statement* effective = GetFirstEffectiveStatement(stmt->body())) {
            for (AVal::ObjectSet::const_iterator it = res.objects().begin(),
                 last = res.objects().end(); it != last; ++it) {
              AObject* obj = *it;
              for (AObject::Properties::const_iterator it2 = obj->properties().begin(),
                   last2 = obj->properties().end(); it2 != last2; ++it2) {
                if (it2->second.IsEnumerable() &&
                    already.find(it2->first) == already.end()) {
                  already.insert(it2->first);
                  CurrentFrame()->Set(
                      heap_, binding, AVal(GetSymbolString(it2->first)));
                  // effective->Accept(this);
                  Interpret(effective, stmt->end());
                }
              }
              if (already.size() > 20) {
                // too big... so stop searching
                DebugLog("STOP!!");
                return;
              }
            }
          }
        }
      }
    }
  }
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
    result_.Reset(AVAL_UNDEFINED);
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
  AVal err(result_.result());  // normal result thrown
  // and if expr throw error, merge this
  if (result_.HasException()) {
    err |= result_.exception();
  }
  result_ = Result(AVal(AVAL_NOBASE), err);
}


void Interpreter::Visit(TryStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(DebuggerStatement* stmt) {
  // do nothing
}

void Interpreter::Visit(ExpressionStatement* stmt) {
  // handling expression statement to assign
  Expression* expr = stmt->expr();
  if (expr->AsIdentifier() || expr->AsIdentifierAccess()) {
    if (std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(expr)) {
      //   /** @type {string} */
      //   Test.prototype.test;
      // or
      //   /** @type {string} */
      //   test;
      if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_TYPE)) {
        assert(tag->type());
        tag->type()->Accept(this);
        result_ = Assign(expr, result_);
        return;
      }
    }
  }
  stmt->expr()->Accept(this);
}

// Expressions

void Interpreter::Visit(Assignment* assign) {
  using iv::core::Token;
  const MethodTargetGuard method_target_guard(this, assign);
  const Token::Type op = assign->op();
  if (op == Token::TK_ASSIGN_ADD) {
    // +=
    // + operator is sepcialized for STRING + STRING
    assign->left()->Accept(this);
    const Result lhs = result_;
    assign->right()->Accept(this);
    result_ = Assign(assign, lhs, result_.result());
  } else {
    // others
    assign->right()->Accept(this);
    result_ = Assign(assign, result_, AVal(AVAL_NOBASE));
  }
}

void Interpreter::Visit(BinaryOperation* binary) {
  using iv::core::Token;
  const MethodTargetGuard method_target_guard(this, binary);
  const Token::Type token = binary->op();
  switch (token) {
    case Token::TK_LOGICAL_AND: {  // &&
      binary->left()->Accept(this);
      const Result lr = result_;
      if (!lr.result().IsFalse()) {
        binary->right()->Accept(this);
        AVal res(result_.result());
        AVal err(AVAL_NOBASE);
        bool error_found = false;
        if (lr.HasException()) {
          error_found = true;
          err.Join(lr.exception());
        }
        if (result_.HasException()) {
          error_found = true;
          err.Join(result_.exception());
        }
        if (lr.result().IsTrue()) {
          // true && "STRING" => STR
          result_ = Result(res, err, error_found);
        } else {
          result_ = Result(lr.result() | res, err, error_found);
        }
      }
      break;
    }

    case Token::TK_LOGICAL_OR: {  // ||
      binary->left()->Accept(this);
      const Result lr = result_;
      if (!lr.result().IsTrue()) {
        binary->right()->Accept(this);
        AVal res(result_.result());
        AVal err(AVAL_NOBASE);
        bool error_found = false;
        if (lr.HasException()) {
          error_found = true;
          err.Join(lr.exception());
        }
        if (result_.HasException()) {
          error_found = true;
          err.Join(result_.exception());
        }
        if (lr.result().IsFalse()) {
          // false || "STRING" => STRING
          result_ = Result(res, err, error_found);
        } else {
          result_ = Result(lr.result() | res, err, error_found);
        }
      }
      break;
    }

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
      const Result lr = result_;
      binary->right()->Accept(this);
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (lr.HasException()) {
        error_found = true;
        err.Join(lr.exception());
      }
      if (result_.HasException()) {
        error_found = true;
        err.Join(result_.exception());
      }
      result_ = Result(AVal(AVAL_NUMBER), err, error_found);
      break;
    }

    case Token::TK_ADD: {  // +
      // add STRING + STRING is STRING
      binary->left()->Accept(this);
      const Result lr = result_;
      binary->right()->Accept(this);
      const AVal res(result_.result() + lr.result());
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (lr.HasException()) {
        error_found = true;
        err.Join(lr.exception());
      }
      if (result_.HasException()) {
        error_found = true;
        err.Join(result_.exception());
      }
      result_ = Result(res, err, error_found);
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
      const Result lr = result_;
      binary->right()->Accept(this);
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (lr.HasException()) {
        error_found = true;
        err.Join(lr.exception());
      }
      if (result_.HasException()) {
        error_found = true;
        err.Join(result_.exception());
      }
      result_ = Result(AVal(AVAL_BOOL), err, error_found);
      break;
    }

    case Token::TK_COMMA: {  // ,
      binary->left()->Accept(this);
      const Result lr = result_;
      binary->right()->Accept(this);
      AVal err(AVAL_NOBASE);
      bool error_found = false;
      if (lr.HasException()) {
        error_found = true;
        err.Join(lr.exception());
      }
      if (result_.HasException()) {
        error_found = true;
        err.Join(result_.exception());
      }
      result_ = Result(result_.result(), err, error_found);
      break;
    }

    default: {
      UNREACHABLE();
      break;
    }
  }
}

void Interpreter::Visit(ConditionalExpression* cond) {
  const MethodTargetGuard method_target_guard(this, cond);
  cond->cond()->Accept(this);
  const Result cr = result_;  // cond result
  cond->left()->Accept(this);
  const Result lr = result_;  // left result
  cond->right()->Accept(this);
  const Result rr = result_;  // right result
  // aggregate error and result values
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  if (cr.HasException()) {
    error_found = true;
    err |= cr.exception();
  }
  if (lr.HasException()) {
    error_found = true;
    err |= lr.exception();
  }
  if (rr.HasException()) {
    error_found = true;
    err |= rr.exception();
  }
  if (cr.result().IsTrue()) {
    // true
    if (lr.HasException()) {
      error_found = true;
      err |= lr.exception();
    }
    result_ = Result(lr.result(), err, error_found);
  } else if (cr.result().IsFalse()) {
    // false
    if (rr.HasException()) {
      error_found = true;
      err |= lr.exception();
    }
    result_ = Result(rr.result(), err, error_found);
  } else {
    // indeterminate
    if (lr.HasException()) {
      error_found = true;
      err |= lr.exception();
    }
    if (rr.HasException()) {
      error_found = true;
      err |= rr.exception();
    }
    result_ = Result(
        lr.result() | rr.result(), err, error_found);
  }
}

void Interpreter::Visit(UnaryOperation* unary) {
  // from iv / lv5 teleporter
  using iv::core::Token;
  const MethodTargetGuard method_target_guard(this, unary);
  switch (unary->op()) {
    case Token::TK_DELETE: {
      // if no error occurred, this always returns BOOL / UNDEFINED
      unary->expr()->Accept(this);
      result_.set_result(AVAL_BOOL | AVAL_UNDEFINED);
      break;
    }

    case Token::TK_VOID: {
      // if no error occurred, this always returns UNDEFINED
      unary->expr()->Accept(this);
      result_.set_result(AVAL_UNDEFINED);
      break;
    }

    case Token::TK_TYPEOF: {
      // if no error occurred, this always returns STRING
      unary->expr()->Accept(this);
      result_.set_result(AVAL_STRING);
      break;
    }

    case Token::TK_INC:
    case Token::TK_DEC:
    case Token::TK_ADD:
    case Token::TK_SUB:
    case Token::TK_BIT_NOT: {
      // if no error occurred, this always returns NUMBER
      unary->expr()->Accept(this);
      result_.set_result(AVAL_NUMBER);
      break;
    }

    case Token::TK_NOT: {
      unary->expr()->Accept(this);
      if (result_.result().IsTrue()) {
        result_.set_result(AVAL_TRUE);
      } else if (result_.result().IsFalse()) {
        result_.set_result(AVAL_FALSE);
      } else {
        // indeterminate
        result_.set_result(AVAL_BOOL);
      }
      break;
    }

    default:
      UNREACHABLE();
  }
}

void Interpreter::Visit(PostfixExpression* postfix) {
  const MethodTargetGuard method_target_guard(this, postfix);
  postfix->expr()->Accept(this);
  result_.set_result(AVAL_NUMBER);
}

void Interpreter::Visit(StringLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_ = Result(AVal(literal->value()));
}

void Interpreter::Visit(NumberLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_.Reset(AVAL_NUMBER);
}

void Interpreter::Visit(Identifier* ident) {
  // lookup!
  const MethodTargetGuard method_target_guard(this, ident);
  Binding* binding = ident->refer();
  if (binding) {
    if (ident->refer()->type() == Binding::HEAP) {
      // this is heap variable, so lookup from heap
      result_ = Result(binding->value());
    } else {
      // this is stack variable, so lookup from frame
      result_ = Result(CurrentFrame()->Get(heap_, binding));
    }
  } else {
    // not found => global lookup
    result_ = Result(
        heap_->GetGlobal().GetProperty(heap_, ident->symbol()));
  }
}

void Interpreter::Visit(ThisLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_ = Result(CurrentFrame()->GetThis());
}

void Interpreter::Visit(NullLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_.Reset(AVAL_NULL);
}

void Interpreter::Visit(TrueLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_.Reset(AVAL_TRUE);
}

void Interpreter::Visit(FalseLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_.Reset(AVAL_FALSE);
}

void Interpreter::Visit(RegExpLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_ = Result(AVal(heap_->GetDeclObject(literal)));
}

void Interpreter::Visit(ArrayLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  AObject* ary = heap_->GetDeclObject(literal);
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  uint32_t index = 0;
  for (MaybeExpressions::const_iterator it = literal->items().begin(),
       last = literal->items().end(); it != last; ++it, ++index) {
    if (const iv::core::Maybe<Expression> expr = *it) {
      expr.Address()->Accept(this);
      ary->UpdateProperty(heap_, Intern(index), result_.result());
      if (result_.HasException()) {
        // error found
        error_found = true;
        err |= result_.exception();
      }
    }
  }
  result_ = Result(AVal(ary), err, error_found);
}

void Interpreter::Visit(ObjectLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  AObject* obj = heap_->GetDeclObject(literal);
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  for (ObjectLiteral::Properties::const_iterator it = literal->properties().begin(),
       last = literal->properties().end(); it != last; ++it) {
    const ObjectLiteral::Property& prop = *it;
    if (std::get<0>(prop) == ObjectLiteral::DATA) {
      std::get<2>(prop)->Accept(this);
      Identifier* ident = std::get<1>(prop);
      obj->UpdateProperty(heap_, ident->symbol(), result_.result());
      if (result_.HasException())  {
        // error found
        error_found = true;
        err |= result_.exception();
      }
    }
  }
  result_ = Result(AVal(obj), err, error_found);
}

void Interpreter::Visit(FunctionLiteral* literal) {
  const MethodTargetGuard method_target_guard(this, literal);
  result_ = Result(AVal(heap_->GetDeclObject(literal)));
}

void Interpreter::Interpret(FunctionLiteral* literal) {
  // interpret function
  // BindingResolver already marks normal / raised continuation
  // so this function use this and walking flow and evaluate this.
  Interpret(literal->normal(), NULL);
  if (heap_->completer() &&
      literal == heap_->completer()->GetTargetFunction()) {
    GainCompletion(heap_->completer());
  }
}

void Interpreter::Interpret(Statement* start, Statement* end) {
  AVal result(AVAL_NOBASE);
  AVal error(AVAL_NOBASE);
  bool error_found = false;
  Tasks tasks;

  tasks.push_back(start);
  while (true) {
    if (tasks.empty()) {
      // all task is done!
      break;
    }
    Statement* const task = tasks.back();
    tasks.pop_back();
    if (!task || task == end) {
      continue;  // next statement
    }

    // patching phase
    if (task->AsReturnStatement()) {
      task->Accept(this);
      result |= result_.result();
    } else {
      task->Accept(this);
    }
    tasks.push_back(task->normal());

    // check answer value and determine evaluate raised path or not
    if (result_.HasException() && !end) {
      error_found = true;
      // raised path is catch / finally / NULL
      // if catch   ... TryStatement,
      //    finally ... Block
      if (task->raised()) {  // if raised path is found
        if (TryStatement* raised = task->raised()->AsTryStatement()) {
          error_found = false;
          assert(raised->catch_name() && raised->catch_block());
          Binding* binding = raised->catch_name().Address()->refer();
          assert(binding);
          if (CurrentFrame()->IsDefined(heap_, binding)) {
            CurrentFrame()->Set(
                heap_,
                binding,
                result_.exception() | CurrentFrame()->Get(heap_, binding));
          } else {
            CurrentFrame()->Set(heap_, binding, result_.exception());
          }
          tasks.push_back(raised->catch_block().Address());
        } else {
          tasks.push_back(task->raised());
        }
      } else {
        error |= result_.exception();
      }
    }
  }
  result_ = Result(result, error, error_found);
}

void Interpreter::Visit(IdentifierAccess* prop) {
  const MethodTargetGuard method_target_guard(this, prop);
  prop->target()->Accept(this);
  base_ = result_.result().ToObject(heap_);
  const AVal refer = base_.GetProperty(heap_, prop->key()->symbol());
  result_.set_result(refer);
}

void Interpreter::Visit(IndexAccess* prop) {
  const MethodTargetGuard method_target_guard(this, prop);
  prop->target()->Accept(this);
  const Result target(result_);
  prop->key()->Accept(this);
  const Result key(result_);
  Result res;
  bool error_found = target.HasException() || key.HasException();
  const AVal keyr(key.result());
  base_ = target.result().ToObject(heap_);
  if (keyr.HasString()) {
    if (keyr.GetStringValue()) {
      res.set_result(base_.GetProperty(heap_, Intern(*keyr.GetStringValue())));
    } else {
      res.set_result(base_.GetStringProperty(heap_));
    }
  } else if (keyr.HasNumber()) {
    res.set_result(base_.GetNumberProperty(heap_));
  } else {
    res.set_result(AVAL_NOBASE);
  }
  if (error_found) {
    AVal ex(AVAL_NOBASE);
    if (target.HasException()) {
      ex |= target.exception();
    }
    if (key.HasException()) {
      ex |= key.exception();
    }
    res.set_exception(ex);
  }
  result_ = res;
}

void Interpreter::Visit(FunctionCall* call) {
  const MethodTargetGuard method_target_guard(this, call);
  call->target()->Accept(this);
  AVal this_binding(AVAL_NOBASE);
  if (call->target()->AsPropertyAccess()) {
    // get base binding
    this_binding = base_;
  } else {
    this_binding = heap_->GetGlobal();
  }
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  if (result_.HasException()) {
    error_found = true;
    err |= result_.exception();
  }
  const AVal target = result_.result();
  // fill arguments
  std::vector<AVal> args(call->args().size(), AVal(AVAL_NOBASE));
  std::vector<AVal>::iterator args_it = args.begin();
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it, ++args_it) {
    (*it)->Accept(this);
    args_it->Join(result_.result());
    if (result_.HasException()) {
      error_found = true;
      err |= result_.exception();
    }
  }
  target.Call(heap_, this, this_binding, args, &result_);
  if (error_found) {
    result_.MergeException(err);
  }
}

void Interpreter::Visit(ConstructorCall* call) {
  const MethodTargetGuard method_target_guard(this, call);
  call->target()->Accept(this);
  AVal err(AVAL_NOBASE);
  bool error_found = false;
  if (result_.HasException()) {
    error_found = true;
    err |= result_.exception();
  }
  const AVal target = result_.result();
  // fill arguments
  std::vector<AVal> args(call->args().size(), AVal(AVAL_NOBASE));
  std::vector<AVal>::iterator args_it = args.begin();
  for (Expressions::const_iterator it = call->args().begin(),
       last = call->args().end(); it != last; ++it, ++args_it) {
    (*it)->Accept(this);
    args_it->Join(result_.result());
    if (result_.HasException()) {
      error_found = true;
      err |= result_.exception();
    }
  }
  target.Construct(heap_, this, heap_->GetDeclObject(call), args, &result_);
  if (error_found) {
    result_.MergeException(err);
  }
}

// Others

void Interpreter::Visit(Declaration* dummy) {
}

Result Interpreter::EvaluateFunction(AObject* function,
                                     const AVal& this_binding,
                                     const std::vector<AVal>& args,
                                     bool IsConstructorCalled) {
  // if builtin function
  if (function->builtin()) {
    return function->builtin()(heap_, this_binding, args, IsConstructorCalled);
  }

  // if JS AST function
  FunctionLiteral* literal = function->function();

  Result res;
  if (heap_->FindSummary(function, this_binding, args, &res)) {
    // summary found. so, use this result
    return res;
  }

  // jsdoc is found?
  std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(literal);

  // interpret function
  std::shared_ptr<Heap::Execution> current;
  while (true) {
    try {
      State start_state = heap_->state();
      std::shared_ptr<Heap::Execution> prev =
          heap_->SearchWaitingResults(literal, this_binding, args);
      if (!prev) {
        // this is first time call
        // so, add it to execution queue
        current = heap_->AddWaitingResults(literal, this_binding, args, false);
      } else if (std::get<2>(*prev) == heap_->state()) {
        // state is not changed
        // first time, through it.
        // second time, shut out this path and return NOBASE
        if (std::get<3>(*prev)) {
          if (info) {
            if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_RETURN)) {
              // @return found, so use it
              assert(tag->type());
              tag->type()->Accept(this);
              return result_;
            }
          }
          return Result(AVal(AVAL_NOBASE));
        } else {
          current = heap_->AddWaitingResults(literal, this_binding, args, true);
        }
      } else {
        // waiting result is too old...
        throw UnwindStack(literal, this_binding, args);
      }

      Frame frame(this);

      CurrentFrame()->SetThis(this_binding);
      const FunctionLiteral::DeclType type = literal->type();
      if (type == FunctionLiteral::STATEMENT ||
          (type == FunctionLiteral::EXPRESSION && literal->name())) {
        // in scope, so set to the frame
        Identifier* name = literal->name().Address();
        Binding* binding = name->refer();
        assert(binding);
        CurrentFrame()->Set(heap_, binding,
                            AVal(heap_->GetDeclObject(literal)));
      }

      // parameter binding initialization
      std::size_t index = 0;
      const std::size_t args_size = args.size();
      for (Identifiers::const_iterator it = literal->params().begin(),
           last = literal->params().end(); it != last; ++it, ++index) {
        Binding* binding = (*it)->refer();
        assert(binding);
        const AVal target((args_size > index) ?
                          args[index] : AVal(AVAL_UNDEFINED));
        if (binding->type() == Binding::HEAP) {
          heap_->UpdateHeap(binding, target);
        }
        CurrentFrame()->Set(heap_, binding, target);
      }

      if (index < args_size) {
        AVal result(AVAL_NOBASE);
        for (;index < args_size; ++index) {
          result.Join(args[index]);
        }
        CurrentFrame()->SetRest(result);
      }

      const Scope& scope = literal->scope();
      for (Scope::Variables::const_iterator it = scope.variables().begin(),
           last = scope.variables().end(); it != last; ++it) {
        const Scope::Variable& var = *it;
        Identifier* ident = var.first;
        Binding* binding = ident->refer();
        assert(binding);
        CurrentFrame()->Set(heap_, binding, AVal(AVAL_NOBASE));
      }

      for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
           last = scope.function_declarations().end(); it != last; ++it) {
        Identifier* ident = (*it)->name().Address();
        Binding* binding = ident->refer();
        assert(binding);
        CurrentFrame()->Set(heap_, binding, AVal(heap_->GetDeclObject(*it)));
      }

      // then, interpret
      Interpret(literal);

      if (IsConstructorCalled) {
        if (result_.result() == AVal(AVAL_NOBASE)) {
          // not nobase => no return statement found
          result_.set_result(this_binding);
        } else {
          // return statement enabled
          // but, if no object found, use this value
          if (result_.result().objects().empty()) {
            result_.set_result(this_binding);
          }
        }
      } else {
        // return only
        // if result value is NOBASE (no return statement),
        // add undefined
        if (result_.result() == AVal(AVAL_NOBASE)) {
          result_.set_result(AVAL_UNDEFINED);
        }
      }

      if (info) {
        if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_RETURN)) {
          // @return found, so use it
          Result current = result_;
          assert(tag->type());
          tag->type()->Accept(this);
          current.MergeResult(result_.result());
          result_ = current;
        }
      }
      heap_->RemoveWaitingResults(literal);
      heap_->AddSummary(function, start_state, this_binding, args, result_);
      return result_;
    } catch (const UnwindStack& ex) {
      if (!current) {
        // first case
        throw ex;
      }
      if (ex.literal() == literal &&
          ex.this_binding() == this_binding &&
          ex.args().size() == args.size() &&
          std::equal(args.begin(), args.end(), ex.args().begin())) {
        if (std::get<3>(*heap_->RemoveWaitingResults(literal))) {
          throw ex;
        }
      } else {
        heap_->RemoveWaitingResults(literal);
        throw ex;
      }
    }
  }
}

Result Interpreter::Assign(Expression* lhs, Result res) {
  if (Identifier* ident = lhs->AsIdentifier()) {
    // Identifier
    Binding* binding = ident->refer();
    if (binding) {
      if (binding->type() == Binding::HEAP) {
        // heap
        heap_->UpdateHeap(binding, res.result());
      } else {
        // stack
        const AVal val(CurrentFrame()->Get(heap_, binding) | res.result());
        CurrentFrame()->Set(heap_, binding, val);
        if (heap_->IsDeclaredHeapBinding(binding)) {
          // global binding
          heap_->UpdateHeap(binding, val);
          heap_->GetGlobal().UpdateProperty(heap_, ident->symbol(), val);
        }
      }
    } else {
      // global assignment
      heap_->GetGlobal().UpdateProperty(heap_, ident->symbol(), res.result());
    }
    return res;
  } else if (lhs->AsCall()) {
    // code like:
    // func() = 20;
    //
    // this is not invalid code,
    // but generally raise ReferenceError
    //
    // TODO(Constellation) ReferenceError AVal required
    lhs->Accept(this);
    return res;
  } else {
    assert(lhs->AsPropertyAccess());
    if (IdentifierAccess* identac = lhs->AsIdentifierAccess()) {
      const Symbol key = identac->key()->symbol();
      identac->target()->Accept(this);
      const Result target(result_);
      target.result().UpdateProperty(heap_, key, res.result());
      res.MergeException(target);
      return res;
    } else {
      assert(lhs->AsIndexAccess());
      IndexAccess* indexac = lhs->AsIndexAccess();
      indexac->target()->Accept(this);
      const Result target(result_);
      indexac->key()->Accept(this);
      const Result key(result_);
      const AVal keyr(key.result());
      if (keyr.HasNumber()) {
        // update number property
        target.result().UpdateNumberProperty(heap_, res.result());
      }
      if (keyr.HasString()) {
        // update string property
        if (keyr.GetStringValue()) {
          target.result().UpdateProperty(
              heap_,
              Intern(*keyr.GetStringValue()),
              res.result());
        } else {
          target.result().UpdateStringProperty(heap_, res.result());
        }
      }
      return res;
    }
  }
  return res;
}

Result Interpreter::Assign(Assignment* assign, Result res, AVal old) {
  // LHS is following pattern
  //   + Call
  //     - FunctionCall
  //     - ConstructorCall
  //   + PropertyAccess
  //     - IndexAccess
  //     - IdentifierAccess
  //   + Identifier
  Expression* lhs = assign->left();
  assert(lhs->IsValidLeftHandSide());
  if (assign->op() != iv::core::Token::TK_ASSIGN) {
    if (assign->op() == iv::core::Token::TK_ASSIGN_ADD) {
      res.set_result(res.result() + old);
    } else {
      res.set_result(AVAL_NUMBER);
    }
  }
  return Assign(lhs, res);
}

void Interpreter::GainCompletion(Completer* completer) {
  completer->GetTargetExpression()->Accept(this);
  result_.result().ToObject(heap_).Complete(heap_, completer);
}

void Interpreter::EvaluateCompletionTargetFunction(Completer* completer) {
  FunctionLiteral* literal = completer->GetTargetFunction();
  const std::vector<AVal> args(literal->params().size(), AVal(AVAL_NOBASE));
  const AVal this_binding(heap_->MakeObject());
  const bool IsConstructorCalled = false;

  std::shared_ptr<Heap::Execution> current;
  while (true) {
    try {
      std::shared_ptr<Heap::Execution> prev =
          heap_->SearchWaitingResults(literal, this_binding, args);
      if (!prev) {
        // this is first time call
        // so, add it to execution queue
        current = heap_->AddWaitingResults(literal, this_binding, args, false);
      } else if (std::get<2>(*prev) == heap_->state()) {
        // state is not changed
        // first time, through it.
        // second time, shut out this path and return NOBASE
        if (std::get<3>(*prev)) {
          return;
        } else {
          current = heap_->AddWaitingResults(literal, this_binding, args, true);
        }
      } else {
        // waiting result is too old...
        throw UnwindStack(literal, this_binding, args);
      }

      Frame frame(this);

      CurrentFrame()->SetThis(this_binding);
      const FunctionLiteral::DeclType type = literal->type();
      if (type == FunctionLiteral::STATEMENT ||
          (type == FunctionLiteral::EXPRESSION && literal->name())) {
        // in scope, so set to the frame
        Identifier* name = literal->name().Address();
        Binding* binding = name->refer();
        assert(binding);
        CurrentFrame()->Set(heap_,
                            binding, AVal(heap_->GetDeclObject(literal)));
      }

      // parameter binding initialization
      std::size_t index = 0;
      const std::size_t args_size = args.size();
      for (Identifiers::const_iterator it = literal->params().begin(),
           last = literal->params().end(); it != last; ++it, ++index) {
        Binding* binding = (*it)->refer();
        assert(binding);
        const AVal target((args_size > index) ?
                          args[index] : AVal(AVAL_UNDEFINED));
        if (binding->type() == Binding::HEAP) {
          heap_->UpdateHeap(binding, target);
        }
        CurrentFrame()->Set(heap_, binding, target);
      }

      if (index < args_size) {
        AVal result(AVAL_NOBASE);
        for (;index < args_size; ++index) {
          result.Join(args[index]);
        }
        CurrentFrame()->SetRest(result);
      }

      const Scope& scope = literal->scope();
      for (Scope::Variables::const_iterator it = scope.variables().begin(),
           last = scope.variables().end(); it != last; ++it) {
        const Scope::Variable& var = *it;
        Identifier* ident = var.first;
        Binding* binding = ident->refer();
        assert(binding);
        CurrentFrame()->Set(heap_, binding, AVal(AVAL_NOBASE));
      }

      for (Scope::FunctionLiterals::const_iterator it = scope.function_declarations().begin(),
           last = scope.function_declarations().end(); it != last; ++it) {
        Identifier* ident = (*it)->name().Address();
        Binding* binding = ident->refer();
        assert(binding);
        CurrentFrame()->Set(heap_, binding, AVal(heap_->GetDeclObject(*it)));
      }

      // then, interpret
      Interpret(literal);

      if (IsConstructorCalled) {
        if (result_.result() == AVal(AVAL_NOBASE)) {
          // not nobase => no return statement found
          result_.set_result(this_binding);
        } else {
          // return statement enabled
          // but, if no object found, use this value
          if (result_.result().objects().empty()) {
            result_.set_result(this_binding);
          }
        }
      } else {
        // return only
        // if result value is NOBASE (no return statement),
        // add undefined
        if (result_.result() == AVal(AVAL_NOBASE)) {
          result_.set_result(AVAL_UNDEFINED);
        }
      }

      heap_->RemoveWaitingResults(literal);
      return;
    } catch (const UnwindStack& ex) {
      if (!current) {
        // first case
        throw ex;
      }
      if (ex.literal() == literal &&
          ex.this_binding() == this_binding &&
          ex.args().size() == args.size() &&
          std::equal(args.begin(), args.end(), ex.args().begin())) {
        if (std::get<3>(*heap_->RemoveWaitingResults(literal))) {
          throw ex;
        }
      } else {
        heap_->RemoveWaitingResults(literal);
        throw ex;
      }
    }
  }
}

Statement* Interpreter::GetFirstEffectiveStatement(Statement* target) {
  while (true) {
    if (Block* block = target->AsBlock()) {
      if (block->body().empty()) {
        return NULL;
      } else {
        target = block->body().front();
      }
    } else {
      return target;
    }
  }
}

void Interpreter::Visit(jsdoc::PrefixQuestionExpression* node) {
  node->expr()->Accept(this);
  result_.MergeResult(AVAL_NULL);
}

void Interpreter::Visit(jsdoc::PrefixBangExpression* node) {
  node->expr()->Accept(this);
  AVal res = result_.result();
  res.ExcludeBase(AVAL_NULL);
  result_.set_result(res);
}

void Interpreter::Visit(jsdoc::PostfixQuestionExpression* node) {
  node->expr()->Accept(this);
  result_.MergeResult(AVAL_NULL);
}

void Interpreter::Visit(jsdoc::PostfixBangExpression* node) {
  node->expr()->Accept(this);
  AVal res = result_.result();
  res.ExcludeBase(AVAL_NULL);
  result_.set_result(res);
}

void Interpreter::Visit(jsdoc::QuestionLiteral* node) {
  result_.Reset(AVAL_NOBASE);
}

void Interpreter::Visit(jsdoc::StarLiteral* node) {
  result_.Reset(AVAL_NOBASE);
}

void Interpreter::Visit(jsdoc::NullLiteral* node) {
  result_.Reset(AVAL_NULL);
}

void Interpreter::Visit(jsdoc::UndefinedLiteral* node) {
  result_.Reset(AVAL_UNDEFINED);
}

void Interpreter::Visit(jsdoc::VoidLiteral* node) {
  result_.Reset(AVAL_UNDEFINED);
}

void Interpreter::Visit(jsdoc::UnionType* node) {
  AVal res(AVAL_NOBASE);
  jsdoc::TypeExpressions* exprs = node->exprs();
  assert(!exprs->empty());
  for (jsdoc::TypeExpressions::const_iterator it = exprs->begin(),
       last = exprs->end(); it != last; ++it) {
    (*it)->Accept(this);
    res |= result_.result();
  }
  result_.set_result(res);
}

void Interpreter::Visit(jsdoc::ArrayType* node) {
  AObject* ary = heap_->GetDeclObject(node);
  if (ary) {
    // already created, so use it
    result_ = Result(AVal(ary));
    return;
  }
  ary = heap_->GetFactory()->NewAObject();
  heap_->DeclObject(node, ary);
  const std::vector<AVal> args;
  ARRAY_CONSTRUCTOR(heap_, AVal(ary), args, true);

  assert(ary);
  uint32_t index = 0;
  for (jsdoc::TypeExpressions::const_iterator it = node->exprs()->begin(),
       last = node->exprs()->end(); it != last; ++it, ++index) {
    (*it)->Accept(this);
    ary->UpdateProperty(heap_, Intern(index), result_.result());
  }
  result_ = Result(AVal(ary));
}

void Interpreter::Visit(jsdoc::RecordType* node) {
  AObject* obj = heap_->GetDeclObject(node);
  if (obj) {
    // already created, so use it
    result_ = Result(AVal(obj));
    return;
  }
  obj = heap_->MakeObject();
  heap_->DeclObject(node, obj);

  assert(obj);
  for (jsdoc::FieldTypes::const_iterator it = node->exprs()->begin(),
       last = node->exprs()->end(); it != last; ++it) {
    jsdoc::FieldType* field = *it;
    if (field->HasValue()) {
      field->value()->Accept(this);
      obj->UpdateProperty(heap_, Intern(*field->key()), result_.result());
    } else {
      obj->UpdateProperty(heap_, Intern(*field->key()), AVal(AVAL_NOBASE));
    }
  }
  result_ = Result(AVal(obj));
}

void Interpreter::Visit(jsdoc::FieldType* node) {
  // TODO(Constellation) implement it
  UNREACHABLE();
}

void Interpreter::Visit(jsdoc::FunctionType* node) {
  // TODO(Constellation) implement it
  result_.Reset();
}

void Interpreter::Visit(jsdoc::NameExpression* node) {
  jsdoc::NameString* str = node->value();
  // primitive type check phase
  if (IsEqualIgnoreCase(*str, "string")) {
    result_.set_result(AVAL_STRING);
  } else if (IsEqualIgnoreCase(*str, "number")) {
    result_.set_result(AVAL_NUMBER);
  } else if (IsEqualIgnoreCase(*str, "boolean")) {
    result_.set_result(AVAL_BOOL);
  } else {
    // class lookup
    // see type registry
    if (FunctionLiteral* literal = heap_->registry()->GetRegisteredConstructorOrInterface(*node->value())) {
      const std::vector<AVal> args;
      AObject* this_binding = heap_->GetDeclObject(node);
      if (!this_binding) {
        this_binding = heap_->MakeObject();
        heap_->DeclObject(node, this_binding);
      }
      AObject* target = heap_->GetDeclObject(literal);
      this_binding->UpdatePrototype(heap_,
                                    target->GetProperty(Intern("prototype")));
      // jsdoc @extends / @interface check
      // TODO(Constellation) lookup only prototype and set
      if (std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(literal)) {
        if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_IMPLEMENTS)) {
          tag->type()->Accept(this);
          this_binding->UpdatePrototype(heap_, result_.result());
        }
        if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_EXTENDS)) {
          tag->type()->Accept(this);
          this_binding->UpdatePrototype(heap_, result_.result());
        }
      }
      // not call FunctionLiteral
      // because FunctionLiteral parses TypeExpression @return, so may recur
      result_.set_result(AVal(this_binding));
    } else {
      // search target in current scope
      // TODO(Constellation) implement it
      result_.set_result(AVAL_NOBASE);
    }
  }
}

void Interpreter::Visit(jsdoc::TypeNameWithApplication* node) {
  // TODO:(Constellation) use application data
  node->expr()->Accept(this);
}

void Interpreter::Visit(jsdoc::ParametersType* node) {
  UNREACHABLE();
}

void Interpreter::Visit(jsdoc::RestExpression* node) {
  UNREACHABLE();
}

void Interpreter::Visit(jsdoc::PostfixEqualExpression* node) {
  UNREACHABLE();
}

} }  // namespace az::cfa2
#endif  // AZ_CFA2_INTERPRETER_H_
