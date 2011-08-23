#ifndef _AZ_JSDOC_FWD_H_
#define _AZ_JSDOC_FWD_H_
#include <iv/stringpiece.h>
namespace az {
namespace jsdoc {

class Parser;
class Info;
class Token;
typedef std::array<iv::core::StringPiece, 20> TagArray;
class Tag;

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_FWD_H_
