#ifndef AZ_ENVIRONMENT_H_
#define AZ_ENVIRONMENT_H_
#include <bitset>
#include <iv/detail/memory.h>
#include <iv/ustringpiece.h>
#include <az/symbol.h>
#include <az/ast_fwd.h>
#include <az/variable_type.h>
namespace az {

class Environment : public std::enable_shared_from_this<Environment> {
 public:
  struct GlobalTag { };
  struct DeclarativeTag { };
  struct ObjectTag { };
  enum Status {
    OBJECT_ENV = 0,
    DECLARATIVE_ENV = 1,
    GLOBAL = 2,
    EVAL = 3,
    ARGUMENTS = 4
  };

  enum TrapStatus {
    VARIABLE_NOT_FOUND = 0,
    VARIABLE_FOUND = 1,
    TRAP_ONLY = 2
  };

  // Global Environment
  static std::shared_ptr<Environment> CreateGlobal() {
    std::shared_ptr<Environment> env(new Environment(GlobalTag()));
    return env;
  }

  // FunctionLiteral / TryStatement (Catch Block) create new DeclarativeEnvironment
  std::shared_ptr<Environment> CreateDeclarativeEnvironment(const AstNode* node) {
    std::shared_ptr<Environment> env(new Environment(shared_from_this(), DeclarativeTag()));
    RegisterNestedEnvironment(node, env);
    return env;
  }

  // WithStatement creates new ObjectEnvironment
  std::shared_ptr<Environment> CreateObjectEnvironment(const AstNode* node) {
    std::shared_ptr<Environment> env(new Environment(shared_from_this(), ObjectTag()));
    RegisterNestedEnvironment(node, env);
    return env;
  }

  std::weak_ptr<Environment> GetUpperEnvironment() const {
    return upper_;
  }

  bool IsGlobal() const {
    return flags_[GLOBAL];
  }

  bool IsObjectEnvironment() const {
    return flags_[OBJECT_ENV];
  }

  bool IsDeclarativeEnvironment() const {
    return flags_[DECLARATIVE_ENV];
  }

  // returns this variable name is duplicate
  bool Instantiate(Symbol sym) {
    const std::pair<VariableMap::iterator, bool> v =
        variables_.insert(VariableMap::value_type(sym, VariableMap::mapped_type()));
    return !v.second;
  }

  std::shared_ptr<Environment> Lookup(Symbol sym) {
    std::shared_ptr<Environment> env = shared_from_this();
    while (env) {
      if (env->IsTrapped(sym)) {
        return env;
      }
      env = env->GetUpperEnvironment().lock();
    }
    return env;
  }

  Var& Get(Symbol sym) {
    return variables_[sym];
  }

  TrapStatus IsTrapped(Symbol sym) const {
    if (variables_.find(sym) != variables_.end()) {
      // variable is found
      return VARIABLE_FOUND;
    } else {
      if (HasEval() || IsObjectEnvironment()) {
        // this traps all, but it maybe be not allocated at variables.
        return TRAP_ONLY;
      } else {
        return VARIABLE_NOT_FOUND;
      }
    }
  }

  bool HasEval() const {
    // TODO(Constellation) implement it
    return false;
  }

 private:
  explicit Environment(GlobalTag tag)
    : variables_(),
      upper_(),
      scopes_(),
      flags_() {
    flags_[OBJECT_ENV] = true;
    flags_[GLOBAL] = true;
  }

  Environment(std::weak_ptr<Environment> upper,
              DeclarativeTag tag)
    : variables_(),
      upper_(upper),
      scopes_(),
      flags_() {
    flags_[DECLARATIVE_ENV] = true;
  }

  Environment(std::weak_ptr<Environment> upper,
              ObjectTag tag)
    : variables_(),
      upper_(upper),
      scopes_(),
      flags_() {
    flags_[OBJECT_ENV] = true;
  }

  void RegisterNestedEnvironment(const AstNode* node, std::shared_ptr<Environment> env) {
    scopes_[node] = env;
  }

  VariableMap variables_;
  std::weak_ptr<Environment> upper_;
  std::unordered_map<const AstNode*, std::shared_ptr<Environment> > scopes_;
  std::bitset<8> flags_;
};

}  // namespace az
#endif  // AZ_ENVIRONMENT_H_
