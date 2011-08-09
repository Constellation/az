#ifndef _AZ_AST_FWD_H_
#define _AZ_AST_FWD_H_
#include <iv/ast.h>
#include <az/cfa2/fwd.h>
#include <az/cfa2/binding.h>
namespace az {

class AstFactory;

}  // namespace az
namespace iv {
namespace core {
namespace ast {

template<>
class AstNodeBase<az::AstFactory>
  : public Inherit<az::AstFactory, kAstNode> {
 public:
  void Location(std::size_t begin, std::size_t end) {
    begin_ = begin;
    end_ = end;
  }
  std::size_t begin_position() const {
    return begin_;
  }
  std::size_t end_position() const {
    return end_;
  }
 private:
  std::size_t begin_;
  std::size_t end_;
};

template<>
class IdentifierBase<az::AstFactory>
  : public Inherit<az::AstFactory, kIdentifier> {
 public:
  void set_type(core::Token::Type type) {
    type_ = type;
  }

  core::Token::Type type() const {
    return type_;
  }

  void set_refer(az::cfa2::Binding* binding) {
    refer_ = binding;
  }

  az::cfa2::Binding* refer() const {
    return refer_;
  }

  void set_binding_type(az::cfa2::Binding::Type type) {
    binding_type_ = type;
  }

  az::cfa2::Binding::Type binding_type() const {
    return binding_type_;
  }
 private:
  core::Token::Type type_;
  az::cfa2::Binding* refer_;
  az::cfa2::Binding::Type binding_type_;
};

template<>
class StatementBase<az::AstFactory>
  : public Inherit<az::AstFactory, kStatement> {
 public:
  typedef Statement<az::AstFactory> StatementType;

  StatementType* normal() const {
    return normal_;
  }

  void set_normal(StatementType* normal) {
    normal_ = normal;
  }

  StatementType* raised() const {
    return raised_;
  }

  void set_raised(StatementType* raised) {
    raised_ = raised;
  }

  void set_is_failed_node(bool val) {
    is_failed_node_ = val;
  }

  bool IsFailed() const {
    return is_failed_node_;
  }

 private:
  StatementType* normal_;
  StatementType* raised_;
  bool is_failed_node_;
};

// stack variable map
template<>
class FunctionLiteralBase<az::AstFactory>
  : public Inherit<az::AstFactory, kFunctionLiteral> {
 public:
  typedef Statement<az::AstFactory> StatementType;

  StatementType* normal() const {
    return normal_;
  }

  void set_normal(StatementType* normal) {
    normal_ = normal;
  }

  StatementType* raised() const {
    return raised_;
  }

  void set_raised(StatementType* raised) {
    raised_ = raised;
  }

 private:
  StatementType* normal_;
  StatementType* raised_;
};

template<>
class BreakableStatementBase<az::AstFactory>
  : public Inherit<az::AstFactory, kBreakableStatement> {
 public:
  typedef Statement<az::AstFactory> StatementType;

  StatementType* jump_to() const {
    return jump_to_;
  }

  void set_jump_to(StatementType* jump_to) {
    jump_to_ = jump_to;
  }

 private:
  StatementType* jump_to_;
};


template<>
class DoWhileStatementBase<az::AstFactory>
  : public Inherit<az::AstFactory, kDoWhileStatement> {
 public:
  typedef ExpressionStatement<az::AstFactory>* ExpressionStatementType;
  void set_cond_statement(ExpressionStatementType cond) {
    cond_statement_ = cond;
  }

  ExpressionStatementType cond_statement() const {
    return cond_statement_;
  }
 private:
  ExpressionStatementType cond_statement_;
};


template<>
class ForStatementBase<az::AstFactory>
  : public Inherit<az::AstFactory, kDoWhileStatement> {
 public:
  typedef ExpressionStatement<az::AstFactory>* ExpressionStatementType;
  void set_cond_statement(ExpressionStatementType cond) {
    cond_statement_ = cond;
  }

  ExpressionStatementType cond_statement() const {
    return cond_statement_;
  }

  void set_next_statement(ExpressionStatementType next) {
    next_statement_ = next;
  }

  ExpressionStatementType next_statement() const {
    return next_statement_;
  }
 private:
  ExpressionStatementType cond_statement_;
  ExpressionStatementType next_statement_;
};

} } }  // namespace iv::core::ast
namespace az {

#define V(AST) typedef iv::core::ast::AST<AstFactory> AST;
  IV_AST_NODE_LIST(V)
#undef V
#define V(XS) typedef iv::core::ast::AstNode<AstFactory>::XS XS;
  IV_AST_LIST_LIST(V)
#undef V
#define V(S) typedef iv::core::SpaceUString<AstFactory>::type S;
  IV_AST_STRING(V)
#undef V
typedef iv::core::ast::AstVisitor<AstFactory>::const_type AstVisitor;
typedef iv::core::ast::AstVisitor<AstFactory>::type MutableAstVisitor;

}  // namespace az
#endif  // _AZ_AST_FWD_H_
