#ifndef _AZ_JSDOC_TAG_H_
#define _AZ_JSDOC_TAG_H_
#include <iv/stringpiece.h>
namespace az {
namespace jsdoc {

typedef std::array<iv::core::StringPiece, 20> TagArray;
static const TagArray kKnownTags = { {
  "@const",
  "@constructor",
  "@define",
  "@deprecated",
  "@enum",
  "@extends",
  "@implements",
  "@inheritDoc",
  "@interface",
  "@license",
  "@preserve",
  "@nosideeffects",
  "@override",
  "@param",
  "@private",
  "@protected",
  "@return",
  "@this",
  "@type",
  "@typedef"
} };

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TAG_H_
