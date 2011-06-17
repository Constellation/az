#ifndef _AZ_ANALYZE_MAP_H_
#define _AZ_ANALYZE_MAP_H_
#include <iv/detail/unordered_map.h>
#include <iv/detail/unordered_set.h>
#include <iv/detail/memory.h>
#include <iv/ustring.h>
#include <iv/noncopyable.h>
#include <utility>
#include "jstype.h"
#include "variable_type.h"
namespace az {

typedef std::unordered_set<JSType> TypeSet;

typedef std::pair<VariableType, TypeSet> Var;
typedef std::unordered_map<iv::core::UString, Var> VariableMap;

typedef std::pair<bool, TypeSet> ReachableAndResult;
typedef std::unordered_map<const Statement*, ReachableAndResult> StatementMap;

}  // namespace az
#endif  // _AZ_ANALYZE_MAP_H_
