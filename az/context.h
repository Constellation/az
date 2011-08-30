#ifndef AZ_CONTEXT_H_
#define AZ_CONTEXT_H_
#include <iv/detail/unordered_map.h>
#include <iv/debug.h>
#include <az/ast_fwd.h>
#include <az/jsdoc/info.h>
namespace az {

class Context : private iv::core::Noncopyable<Context> {
 public:
  typedef std::unordered_map<AstNode*, std::shared_ptr<jsdoc::Info> > JSDocMap;
  Context() : docs_() { }

  virtual ~Context() { }

  void Tag(AstNode* node, const std::shared_ptr<jsdoc::Info>& info) {
    assert(node);
    assert(info);
    docs_[node] = info;
  }

  std::shared_ptr<jsdoc::Info> GetInfo(AstNode* node) {
    JSDocMap::const_iterator it = docs_.find(node);
    if (it != docs_.end()) {
      return it->second;
    }
    return std::shared_ptr<jsdoc::Info>();
  }

 private:
  JSDocMap docs_;
};

}  // namespace az
#endif  // AZ_CONTEXT_H_
