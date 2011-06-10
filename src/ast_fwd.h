#ifndef _AZ_AST_FWD_H_
#define _AZ_AST_FWD_H_
#include <iv/ast.h>
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
 private:
  core::Token::Type type_;
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

}  // namespace az
#endif  // _AZ_AST_FWD_H_
