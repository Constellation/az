#ifndef _AZ_EXECUTION_CONTEXT_H_
#define _AZ_EXECUTION_CONTEXT_H_
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include "ast_fwd.h"
#include "environment.h"
namespace az {

class ExecutionContext : public std::enable_shared_from_this<ExecutionContext> {
 public:

  static std::shared_ptr<ExecutionContext> CreateGlobal(const FunctionLiteral* global) {
    std::shared_ptr<ExecutionContext> ctx(new ExecutionContext(global));
    return ctx;
  }

  std::shared_ptr<ExecutionContext> Create(const FunctionLiteral* literal) {
    std::shared_ptr<ExecutionContext> ctx(new ExecutionContext(shared_from_this(), literal));
    RegisterExecutionContext(literal, ctx);
    return ctx;
  }

  std::shared_ptr<Environment> GetVariableEnvironment() const {
    return variable_environment_;
  }

  std::shared_ptr<Environment> GetLexicalEnvironment() const {
    return lexical_environment_;
  }

  void DownInWithStatement(const WithStatement* stmt) {
    lexical_environment_ = lexical_environment_->CreateObjectEnvironment(stmt);
  }

  void UpFromWithStatement() {
    lexical_environment_ = lexical_environment_->GetUpperEnvironment().lock();
  }

  void DownInCatchBlock(const TryStatement* stmt) {
    lexical_environment_ = lexical_environment_->CreateDeclarativeEnvironment(stmt);
  }

  void UpFromCatchBlock() {
    lexical_environment_ = lexical_environment_->GetUpperEnvironment().lock();
  }

 private:
  // New Function
  ExecutionContext(std::weak_ptr<ExecutionContext> prev, const FunctionLiteral* literal)
    : variable_environment_(prev.lock()->GetLexicalEnvironment()->CreateDeclarativeEnvironment(literal)),
      lexical_environment_(variable_environment_),
      previous_(prev) {
  }

  // Global
  explicit ExecutionContext(const FunctionLiteral* literal)
    : variable_environment_(Environment::CreateGlobal()),
      lexical_environment_(variable_environment_),
      previous_() {
  }

  void RegisterExecutionContext(const FunctionLiteral* node,
                                std::shared_ptr<ExecutionContext> ctx) {
    contexts_[node] = ctx;
  }

  std::shared_ptr<Environment> variable_environment_;
  std::shared_ptr<Environment> lexical_environment_;
  std::weak_ptr<ExecutionContext> previous_;
  std::unordered_map<const FunctionLiteral*, std::shared_ptr<ExecutionContext> > contexts_;
};

}  // namespace az
#endif  // _AZ_EXECUTION_CONTEXT_H_
