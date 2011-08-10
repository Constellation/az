#ifndef _AZ_CFA2_STATE_H_
#define _AZ_CFA2_STATE_H_
#include <iv/detail/cstdint.h>
namespace az {
namespace cfa2 {

typedef uint64_t State;
static const State kInvalidState = 0;
static const State kInitialState = 1;

} }  // namespace az::cfa2
#endif  //_AZ_CFA2_STATE_H_
