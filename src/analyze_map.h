#ifndef _AZ_ANALYZE_MAP_H_
#define _AZ_ANALYZE_MAP_H_
#include <iv/detail/unordered_map.h>
#include <utility>
#include "variable_type.h"
#include "ast_fwd.h"
namespace az {

typedef std::pair<bool, TypeSet> ReachableAndResult;
typedef std::unordered_map<const Statement*, ReachableAndResult> StatementMap;

}  // namespace az
#endif  // _AZ_ANALYZE_MAP_H_
