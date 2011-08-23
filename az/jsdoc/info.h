#ifndef _AZ_JSDOC_INFO_H_
#define _AZ_JSDOC_INFO_H_
#include <vector>
#include <iv/detail/memory.h>
#include <iv/ustringpiece.h>
#include <az/jsdoc/fwd.h>
namespace az {
namespace jsdoc {

class Info {
 public:
  Info()
    : tags_() {
  }

  void Register(const std::shared_ptr<Tag>& tag) {
    tags_.push_back(tag);
  }
 private:
  std::vector<std::shared_ptr<Tag> > tags_;
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_INFO_H_
