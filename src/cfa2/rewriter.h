// rewrite AST to analyze by using CFA2
#ifndef _AZ_CFA2_REWRITER_H_
#define _AZ_CFA2_REWRITER_H_
#include <iv/ast_visitor.h>
#include <iv/maybe.h>
#include <iv/noncopyable.h>

class ReWriter
  : public iv::core::ast::AstVisitor<AstFactory>::type,
    private iv::core::Noncopyable<ReWriter<Reporter> > {
};
#endif  // _AZ_CFA2_REWRITER_H_
