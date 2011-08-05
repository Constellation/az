#ifndef _AZ_CFA2_ANSWER_H_
#define _AZ_CFA2_ANSWER_H_
#include <az/cfa2/fwd.h>
#include <az/cfa2/frame.h>
namespace az {
namespace cfa2 {

typedef std::tuple<AVal, Frame*, bool, AVal> Answer;

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_ANSWER_H_
