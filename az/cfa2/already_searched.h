#ifndef AZ_CFA2_ALREADY_SEARCHED_H_
#define AZ_CFA2_ALREADY_SEARCHED_H_
#include <iv/detail/unordered_map.h>
#include <az/cfa2/fwd.h>
namespace az {
namespace cfa2 {

typedef std::unordered_set<const AObject*> AlreadySearched;

} }  // namespace az::cfa2
#endif  // AZ_CFA2_ALREADY_SEARCHED_H_
