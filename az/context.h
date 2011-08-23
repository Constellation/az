#ifndef _AZ_CONTEXT_H_
#define _AZ_CONTEXT_H_
#include <iv/detail/unordered_map.h>
#include <az/ast_fwd.h>
#include <az/jsdoc/info.h>
namespace az {

class Context : private iv::core::Noncopyable<Context> {
 public:
  Context() : docs_() { }

  virtual ~Context() { }

 private:
  std::unordered_map<AstNode*, std::shared_ptr<jsdoc::Info> > docs_;
};

}  // namespace az
#endif  // _AZ_CONTEXT_H_
