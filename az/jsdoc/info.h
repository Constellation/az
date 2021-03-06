#ifndef AZ_JSDOC_INFO_H_
#define AZ_JSDOC_INFO_H_
#include <vector>
#include <iv/detail/memory.h>
#include <iv/ustringpiece.h>
#include <az/jsdoc/fwd.h>
#include <az/jsdoc/tag.h>
#include <az/jsdoc/token.h>
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

  const std::vector<std::shared_ptr<Tag> > tags() const {
    return tags_;
  }

  // TODO:(Constellation)
  // more efficiency info structure
  bool HasType() const {
    return GetTag(Token::TK_TYPE);
  }

  std::shared_ptr<Tag> GetTag(Token::Type token) const {
    for (std::vector<std::shared_ptr<Tag> >::const_iterator it = tags_.begin(),
         last = tags_.end(); it != last; ++it) {
      if ((*it)->token() == token) {
        return *it;
      }
    }
    return std::shared_ptr<Tag>();
  }

 private:
  std::vector<std::shared_ptr<Tag> > tags_;
};

} }  // namespace az::jsdoc
#endif  // AZ_JSDOC_INFO_H_
