#ifndef _AZ_JSDOC_TYPE_AST_VISITOR_H_
#define _AZ_JSDOC_TYPE_AST_VISITOR_H_
#include <az/jsdoc/type_ast_fwd.h>
namespace az {
namespace jsdoc {

class TypeAstVisitor {
 public:
  virtual ~TypeAstVisitor() { }
#define V(NODE)\
  virtual void Visit(az::jsdoc::NODE* node) = 0;
TYPE_EXPRESSION_AST_DERIVED_NODES(V)
#undef V
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TYPE_AST_VISITOR_H_
