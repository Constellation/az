// simple type expression ast
#ifndef _AZ_JSDOC_TYPE_AST_H_
#define _AZ_JSDOC_TYPE_AST_H_
#include <vector>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/jsdoc/type_ast_fwd.h>
#include <az/jsdoc/type_ast_visitor.h>
namespace az {
namespace jsdoc {

class TypeAstVisitor;

#define ACCEPT_VISITOR\
  inline void Accept(TypeAstVisitor* visitor) {\
    visitor->Visit(this);\
  }

#define DECLARE_NODE_TYPE(type)\
  inline const type* As##type() const { return this; }\
  inline type* As##type() { return this; }

#define DECLARE_DERIVED_NODE_TYPE(type)\
  DECLARE_NODE_TYPE(type)\
  ACCEPT_VISITOR

#define DECLARE_NODE_TYPE_BASE(type)\
  inline virtual const type* As##type() const { return NULL; }\
  inline virtual type* As##type() { return NULL; }

typedef std::vector<TypeExpression*> TypeExpressions;

class TypeExpression : private iv::core::Noncopyable<TypeExpression> {
 public:
  virtual ~TypeExpression() { }
  // TYPE_EXPRESSION_AST_NODES(DECLARE_NODE_TYPE_BASE)
  virtual void Accept(TypeAstVisitor* visitor) { }
};

class PrefixQuestionExpression : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(PrefixQuestionExpression)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class PrefixBangExpression : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(PrefixBangExpression)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class PostfixQuestionExpression : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(PostfixQuestionExpression)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class PostfixBangExpression : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(PostfixBangExpression)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class QuestionLiteral : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(QuestionLiteral)
};

class StarLiteral : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(StarLiteral)
};

class NullLiteral : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(NullLiteral)
};

class UndefinedLiteral : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(UndefinedLiteral)
};

class VoidLiteral : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(VoidLiteral)
};

class UnionType : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(UnionType)
 private:
  std::shared_ptr<TypeExpressions> exprs_;
};

class ArrayType : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(ArrayType)
 private:
  std::shared_ptr<TypeExpressions> exprs_;
};

class RecordType : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(RecordType)
 private:
  std::shared_ptr<TypeExpressions> exprs_;
};

class FunctionType : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(FunctionType)
};

class TypeName : public TypeExpression {
 public:
  DECLARE_NODE_TYPE(TypeName)
};

class NameExpression : public TypeName {
 public:
  DECLARE_DERIVED_NODE_TYPE(NameExpression)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class TypeNameWithApplication : public TypeName {
 public:
  DECLARE_DERIVED_NODE_TYPE(TypeNameWithApplication)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class ParametersType : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(ParametersType)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class RestExpression : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(RestExpression)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

class PostfixEqualExpression : public TypeExpression {
 public:
  DECLARE_DERIVED_NODE_TYPE(PostfixEqualExpression)
 private:
  std::shared_ptr<TypeExpression> expr_;
};

#undef ACCEPT_VISITOR
#undef DECLARE_NODE_TYPE
#undef DECLARE_DERIVED_NODE_TYPE
#undef DECLARE_NODE_TYPE_BASE
} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TYPE_AST_H_
