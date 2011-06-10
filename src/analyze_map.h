#ifndef _AZ_ANALYZE_MAP_H_
#define _AZ_ANALYZE_MAP_H_
#include <iv/detail/unordered_map.h>
#include <iv/detail/unordered_set.h>
#include <iv/detail/memory.h>
#include <iv/ustring.h>
#include <utility>
#include "jstype.h"
#include "variable_type.h"

namespace az {

typedef std::unordered_set<JSType> TypeSet;
typedef std::pair<bool, TypeSet> ReachableAndResult;
typedef std::unordered_map<const Statement*, ReachableAndResult> StatementMap;
typedef std::unordered_map<iv::core::UString, std::pair<VariableType, TypeSet> > VariableMap;
typedef std::pair<VariableMap, StatementMap> FunctionInfo;
typedef std::unordered_map<const FunctionLiteral*, std::shared_ptr<FunctionInfo> > FunctionMap;

}  // namespace az
#endif  // _AZ_ANALYZE_MAP_H_
