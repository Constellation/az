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

  // interpret global function
  for (Statements::const_iterator it = global->body().begin(),
       last = global->body().end(); it != last; ++it) {
    (*it)->Accept(this);
  }

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
}

void Interpreter::Visit(FunctionStatement* func) {
}

void Interpreter::Visit(FunctionDeclaration* func) {
}

void Interpreter::Visit(VariableStatement* var) {
}

void Interpreter::Visit(EmptyStatement* stmt) {
}

void Interpreter::Visit(IfStatement* stmt) {
}

void Interpreter::Visit(DoWhileStatement* stmt) {
}

void Interpreter::Visit(WhileStatement* stmt) {
}

void Interpreter::Visit(ForStatement* stmt) {
}

void Interpreter::Visit(ForInStatement* stmt) {
}

void Interpreter::Visit(ContinueStatement* stmt) {
}

void Interpreter::Visit(BreakStatement* stmt) {
}

void Interpreter::Visit(ReturnStatement* stmt) {
}

void Interpreter::Visit(WithStatement* stmt) {
}

void Interpreter::Visit(LabelledStatement* stmt) {
}

void Interpreter::Visit(SwitchStatement* stmt) {
}

void Interpreter::Visit(ThrowStatement* stmt) {
}

void Interpreter::Visit(TryStatement* stmt) {
}

void Interpreter::Visit(DebuggerStatement* stmt) {
}

void Interpreter::Visit(ExpressionStatement* stmt) {
}

// Expressions

void Interpreter::Visit(Assignment* assign) {
}

void Interpreter::Visit(BinaryOperation* binary) {
}

void Interpreter::Visit(ConditionalExpression* cond) {
}

void Interpreter::Visit(UnaryOperation* unary) {
}

void Interpreter::Visit(PostfixExpression* postfix) {
}

void Interpreter::Visit(StringLiteral* literal) {
}

void Interpreter::Visit(NumberLiteral* literal) {
}

void Interpreter::Visit(Identifier* literal) {
}

void Interpreter::Visit(ThisLiteral* literal) {
}

void Interpreter::Visit(NullLiteral* lit) {
}

void Interpreter::Visit(TrueLiteral* lit) {
}

void Interpreter::Visit(FalseLiteral* lit) {
}

void Interpreter::Visit(RegExpLiteral* literal) {
}

void Interpreter::Visit(ArrayLiteral* literal) {
}

void Interpreter::Visit(ObjectLiteral* literal) {
}

void Interpreter::Visit(FunctionLiteral* literal) {
  // interpret function
  // BindingResolver already marks normal / raised continuation
  // so this function use this and walking flow and evaluate this.
  AVal result(AVAL_NOBASE);
  AVal error(AVAL_NOBASE);
  bool error_found;
  std::deque<Statement*> tasks;
  tasks.push_back(literal->normal());
  while (true) {
    if (tasks.empty()) {
      // all task is done!
      break;
    }
    Statement* const task = tasks.back();
    tasks.pop_back();
    if (!task) {
      continue;  // next statement
    }
    std::cout << "STMT" << std::endl;
    task->Accept(this);
    tasks.push_back(task->normal());
    // check answer value and determine evaluate raised path or not
    if (std::get<1>(answer_)) {
      error_found = true;
      // raised path is catch / finally / NULL
      // if catch   ... TryStatement,
      //    finally ... Block
      if (task->raised()) {  // if raised path is found
        if (TryStatement* raised = task->AsTryStatement()) {
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
        tasks.push_back(task->raised());
      } else {
        error.Join(std::get<2>(answer_));
      }
    }
  }
  answer_ = Answer(result, error_found, error);
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

void Interpreter::Visit(CaseClause* clause) {
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
  Visit(literal);
  heap_->AddSummary(function, this_binding, args, answer_);

  frame_ = previous;

  return result;
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_INTERPRETER_H_
