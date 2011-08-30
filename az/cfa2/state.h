#ifndef AZ_CFA2_STATE_H_
#define AZ_CFA2_STATE_H_
#include <iv/detail/cstdint.h>
namespace az {
namespace cfa2 {

typedef uint64_t State;
static const State kInvalidState = 0;
static const State kInitialState = 1;

} }  // namespace az::cfa2
#endif  //AZ_CFA2_STATE_H_
