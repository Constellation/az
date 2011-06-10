#ifndef _AZ_ANALYZE_MAP_H_
#define _AZ_ANALYZE_MAP_H_
#include <iv/detail/unordered_map.h>
#include <iv/detail/unordered_set.h>
#include <iv/detail/memory.h>
#include <utility>
#include "jstype.h"

namespace az {

typedef std::pair<bool, std::unordered_set<JSType> > ReachableAndResult;
typedef std::unordered_map<Statement*, ReachableAndResult> StatementMap;
typedef std::unordered_map<FunctionLiteral*, std::shared_ptr<StatementMap> > FunctionMap;

}  // namespace az
#endif  // _AZ_ANALYZE_MAP_H_
