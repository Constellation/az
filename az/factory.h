#ifndef AZ_FACTORY_H_
#define AZ_FACTORY_H_
#include <vector>
#include <iv/detail/array.h>
#include <iv/alloc.h>
#include <iv/ustringpiece.h>
#include <iv/space.h>
#include <iv/utils.h>
#include <iv/maybe.h>
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <iv/ast_factory.h>
#include <az/symbol.h>
#include <az/cfa2/binding.h>
namespace az {

class AstFactory
  : public iv::core::ast::BasicAstFactory<AstFactory>,
    public iv::core::Space {
 public:
  typedef iv::core::ast::BasicAstFactory<AstFactory> super_type;
  Scope* NewScope(FunctionLiteral::DeclType type) {
    Scope* scope = new (this) Scope(this, type == FunctionLiteral::GLOBAL);
    scope->literals_ = NewVector<FunctionLiteral*>();
    return scope;
  }

  Assigned* NewAssigned(iv::core::ast::SymbolHolder symbol) {
    Assigned* ident = super_type::NewAssigned(symbol);
    ident->set_refer(NULL);
    ident->set_binding_type(cfa2::Binding::NONE);
    return ident;
  }

  Identifier* NewIdentifier(iv::core::Token::Type type,
                            Symbol symbol,
                            std::size_t begin,
                            std::size_t end) {
    Identifier* ident = super_type::NewIdentifier(type, symbol, begin, end);
    ident->set_type(type);
    ident->set_refer(NULL);
    ident->set_binding_type(cfa2::Binding::NONE);
    return ident;
  }

  NumberLiteral* NewReducedNumberLiteral(const double& val) {
    UNREACHABLE();
    return NULL;
  }

  EmptyStatement* NewEmptyStatement(std::size_t begin, std::size_t end) {
    EmptyStatement* empty = super_type::NewEmptyStatement(begin, end);
    empty->set_is_failed_node(false);
    return empty;
  }

  DebuggerStatement* NewDebuggerStatement(std::size_t begin, std::size_t end) {
    DebuggerStatement* debug = super_type::NewDebuggerStatement(begin, end);
    debug->set_is_failed_node(false);
    return debug;
  }

  FunctionStatement* NewFunctionStatement(FunctionLiteral* func) {
    FunctionStatement* stmt = super_type::NewFunctionStatement(func);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  FunctionDeclaration* NewFunctionDeclaration(FunctionLiteral* func) {
    FunctionDeclaration* decl = super_type::NewFunctionDeclaration(func);
    decl->set_is_failed_node(false);
    return decl;
  }

  Block* NewBlock(Statements* body, std::size_t begin, std::size_t end) {
    Block* block = super_type::NewBlock(body, begin, end);
    block->set_is_failed_node(false);
    return block;
  }

  VariableStatement* NewVariableStatement(iv::core::Token::Type token,
                                          Declarations* decls,
                                          std::size_t begin,
                                          std::size_t end) {
    assert(!decls->empty());
    VariableStatement* var =
        super_type::NewVariableStatement(token, decls, begin, end);
    var->set_is_failed_node(false);
    return var;
  }

  IfStatement* NewIfStatement(Expression* cond,
                              Statement* then_statement,
                              iv::core::Maybe<Statement> else_statement,
                              std::size_t begin) {
    IfStatement* stmt =
        super_type::NewIfStatement(cond, then_statement, else_statement, begin);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  DoWhileStatement* NewDoWhileStatement(Statement* body,
                                        Expression* cond,
                                        std::size_t begin,
                                        std::size_t end) {
    DoWhileStatement* stmt =
        super_type::NewDoWhileStatement(body, cond, begin, end);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  WhileStatement* NewWhileStatement(Statement* body,
                                    Expression* cond,
                                    std::size_t begin) {
    assert(body && cond);
    WhileStatement* stmt =
        super_type::NewWhileStatement(body, cond, begin);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  ForInStatement* NewForInStatement(Statement* body,
                                    Statement* each,
                                    Expression* enumerable,
                                    std::size_t begin) {
    assert(body);
    ForInStatement* stmt =
        super_type::NewForInStatement(body, each, enumerable, begin);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  ForStatement* NewForStatement(Statement* body,
                                iv::core::Maybe<Statement> init,
                                iv::core::Maybe<Expression> cond,
                                iv::core::Maybe<Expression> next,
                                std::size_t begin) {
    assert(body);
    ForStatement* stmt =
        super_type::NewForStatement(body, init, cond, next, begin);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  ExpressionStatement* NewExpressionStatement(Expression* expr, std::size_t end) {
    ExpressionStatement* stmt =
        super_type::NewExpressionStatement(expr, end);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  ContinueStatement* NewContinueStatement(const iv::core::ast::SymbolHolder& label,
                                          IterationStatement** target,
                                          std::size_t begin,
                                          std::size_t end) {
    ContinueStatement* stmt =
        super_type::NewContinueStatement(label, target, begin, end);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  BreakStatement* NewBreakStatement(const iv::core::ast::SymbolHolder& label,
                                    BreakableStatement** target,
                                    std::size_t begin,
                                    std::size_t end) {
    BreakStatement* stmt =
        super_type::NewBreakStatement(label, target, begin, end);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  ReturnStatement* NewReturnStatement(iv::core::Maybe<Expression> expr,
                                      std::size_t begin,
                                      std::size_t end) {
    ReturnStatement* stmt =
        super_type::NewReturnStatement(expr, begin, end);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  WithStatement* NewWithStatement(Expression* expr,
                                  Statement* body, std::size_t begin) {
    WithStatement* stmt =
        super_type::NewWithStatement(expr, body, begin);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  SwitchStatement* NewSwitchStatement(Expression* expr, CaseClauses* clauses,
                                      std::size_t begin, std::size_t end) {
    SwitchStatement* stmt =
        super_type::NewSwitchStatement(expr, clauses, begin, end);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  CaseClause* NewCaseClause(bool is_default,
                            iv::core::Maybe<Expression> expr, Statements* body,
                            std::size_t begin,
                            std::size_t end) {
    CaseClause* clause =
        super_type::NewCaseClause(is_default, expr, body, begin ,end);
    return clause;
  }


  ThrowStatement*  NewThrowStatement(Expression* expr,
                                     std::size_t begin, std::size_t end) {
    ThrowStatement* stmt =
        super_type::NewThrowStatement(expr, begin, end);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  TryStatement* NewTryStatement(Block* try_block,
                                iv::core::Maybe<Assigned> catch_name,
                                iv::core::Maybe<Block> catch_block,
                                iv::core::Maybe<Block> finally_block,
                                std::size_t begin) {
    TryStatement* stmt =
        super_type::NewTryStatement(try_block,
                                    catch_name,
                                    catch_block,
                                    finally_block, begin);
    stmt->set_is_failed_node(false);
    return stmt;
  }

  LabelledStatement* NewLabelledStatement(
      const iv::core::ast::SymbolHolder& label, Statement* stmt) {
    LabelledStatement* res =
        super_type::NewLabelledStatement(label, stmt);
    res->set_is_failed_node(false);
    return res;
  }
};

}  // namespace az
#endif  // AZ_H_
