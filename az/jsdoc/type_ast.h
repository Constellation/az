// simple type expression ast
#ifndef AZ_JSDOC_TYPE_AST_H_
#define AZ_JSDOC_TYPE_AST_H_
#include <iv/noncopyable.h>
#include <az/factory.h>
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

typedef iv::core::SpaceVector<AstFactory, TypeExpression*>::type TypeExpressions;
typedef iv::core::SpaceUString<AstFactory>::type NameString;

class TypeExpression
  : private iv::core::Noncopyable<TypeExpression>,
    public iv::core::SpaceObject {
 public:
  virtual ~TypeExpression() { }
  TYPE_EXPRESSION_AST_NODES(DECLARE_NODE_TYPE_BASE)
  virtual void Accept(TypeAstVisitor* visitor) { }
};

class PrefixQuestionExpression : public TypeExpression {
 public:
  PrefixQuestionExpression(TypeExpression* expr) : expr_(expr) { }
  TypeExpression* expr() const {
    return expr_;
  }
  DECLARE_DERIVED_NODE_TYPE(PrefixQuestionExpression)
 private:
  TypeExpression* expr_;
};

class PrefixBangExpression : public TypeExpression {
 public:
  PrefixBangExpression(TypeExpression* expr) : expr_(expr) { }
  TypeExpression* expr() const {
    return expr_;
  }
  DECLARE_DERIVED_NODE_TYPE(PrefixBangExpression)
 private:
  TypeExpression* expr_;
};

class PostfixQuestionExpression : public TypeExpression {
 public:
  PostfixQuestionExpression(TypeExpression* expr) : expr_(expr) { }
  TypeExpression* expr() const {
    return expr_;
  }
  DECLARE_DERIVED_NODE_TYPE(PostfixQuestionExpression)
 private:
  TypeExpression* expr_;
};

class PostfixBangExpression : public TypeExpression {
 public:
  PostfixBangExpression(TypeExpression* expr) : expr_(expr) { }
  TypeExpression* expr() const {
    return expr_;
  }
  DECLARE_DERIVED_NODE_TYPE(PostfixBangExpression)
 private:
  TypeExpression* expr_;
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
  UnionType(TypeExpressions* exprs) : exprs_(exprs) { }
  TypeExpressions* exprs() const {
    return exprs_;
  }
  DECLARE_DERIVED_NODE_TYPE(UnionType)
 private:
  TypeExpressions* exprs_;
};

class ArrayType : public TypeExpression {
 public:
  ArrayType(TypeExpressions* exprs) : exprs_(exprs) { }
  TypeExpressions* exprs() const {
    return exprs_;
  }
  DECLARE_DERIVED_NODE_TYPE(ArrayType)
 private:
  TypeExpressions* exprs_;
};

class RecordType : public TypeExpression {
 public:
  RecordType(TypeExpressions* exprs) : exprs_(exprs) { }
  TypeExpressions* exprs() const {
    return exprs_;
  }
  DECLARE_DERIVED_NODE_TYPE(RecordType)
 private:
  TypeExpressions* exprs_;
};

class FieldTypeKeyValue : public TypeExpression {
 public:
  FieldTypeKeyValue(TypeExpression* key,
                    TypeExpression* value)
    : key_(key),
      value_(value) {
  }
  TypeExpression* key() const {
    return key_;
  }
  TypeExpression* value() const {
    return value_;
  }
  DECLARE_DERIVED_NODE_TYPE(FieldTypeKeyValue)
 private:
  TypeExpression* key_;
  TypeExpression* value_;
};

class FunctionType : public TypeExpression {
 public:
  FunctionType(bool is_new,
               TypeName* this_binding,
               ParametersType* params,
               TypeExpression* result)
    : is_new_(is_new),
      this_binding_(this_binding),
      params_(params),
      result_(result) {
  }
  bool IsNew() const {
    return is_new_;
  }
  TypeName* this_binding() const {
    return this_binding_;
  }
  ParametersType* params() const {
    return params_;
  }
  TypeExpression* result() const {
    return result_;
  }
  DECLARE_DERIVED_NODE_TYPE(FunctionType)

 private:
  bool is_new_;
  TypeName* this_binding_;
  ParametersType* params_;
  TypeExpression* result_;
};

class TypeName : public TypeExpression {
 public:
  DECLARE_NODE_TYPE(TypeName)
};

class NameExpression : public TypeName {
 public:
  NameExpression(NameString* str) : str_(str) { }
  NameString* value() {
    return str_;
  }
  DECLARE_DERIVED_NODE_TYPE(NameExpression)
 private:
  NameString* str_;
};

class TypeNameWithApplication : public TypeName {
 public:
  TypeNameWithApplication(TypeExpression* expr,
                          TypeExpressions* application)
    : expr_(expr),
      application_(application) {
  }
  TypeExpression* expr() const {
    return expr_;
  }
  TypeExpressions* application() const {
    return application_;
  }
  DECLARE_DERIVED_NODE_TYPE(TypeNameWithApplication)
 private:
  TypeExpression* expr_;
  TypeExpressions* application_;
};

class ParametersType : public TypeExpression {
 public:
  ParametersType(TypeExpressions* exprs) : exprs_(exprs) { }
  TypeExpressions* exprs() const {
    return exprs_;
  }
  DECLARE_DERIVED_NODE_TYPE(ParametersType)
 private:
  TypeExpressions* exprs_;
};

class RestExpression : public TypeExpression {
 public:
  RestExpression(TypeExpression* expr) : expr_(expr) { }
  TypeExpression* expr() const {
    return expr_;
  }
  DECLARE_DERIVED_NODE_TYPE(RestExpression)
 private:
  TypeExpression* expr_;
};

class PostfixEqualExpression : public TypeExpression {
 public:
  PostfixEqualExpression(TypeExpression* expr) : expr_(expr) { }
  TypeExpression* expr() const {
    return expr_;
  }
  DECLARE_DERIVED_NODE_TYPE(PostfixEqualExpression)
 private:
  TypeExpression* expr_;
};

#undef ACCEPT_VISITOR
#undef DECLARE_NODE_TYPE
#undef DECLARE_DERIVED_NODE_TYPE
#undef DECLARE_NODE_TYPE_BASE
} }  // namespace az::jsdoc
#endif  // AZ_JSDOC_TYPE_AST_H_
