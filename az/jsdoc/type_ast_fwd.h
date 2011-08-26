#ifndef _AZ_JSDOC_TYPE_AST_FWD_H_
#define _AZ_JSDOC_TYPE_AST_FWD_H_

#define TYPE_EXPRESSION_AST_DERIVED_NODES(V)\
  V(PrefixQuestionExpression)\
  V(PrefixBangExpression)\
  V(PostfixQuestionExpression)\
  V(PostfixBangExpression)\
  V(QuestionLiteral)\
  V(StarLiteral)\
  V(NullLiteral)\
  V(UndefinedLiteral)\
  V(VoidLiteral)\
  V(UnionType)\
  V(ArrayType)\
  V(RecordType)\
  V(FunctionType)\
  V(NameExpression)\
  V(TypeNameWithApplication)\
  V(ParametersType)\
  V(RestExpression)\
  V(PostfixEqualExpression)

#define TYPE_EXPRESSION_AST_NODES(V)\
  TYPE_EXPRESSION_AST_DERIVED_NODES(V)\
  V(TypeExpression)\
  V(TypeName)

namespace az {
namespace jsdoc {

#define V(NODE) class NODE;
TYPE_EXPRESSION_AST_NODES(V)
#undef V

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TYPE_AST_FWD_H_
