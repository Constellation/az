#ifndef _AZ_CFA2_TYPE_UTILS_H_
#define _AZ_CFA2_TYPE_UTILS_H_
#include <iv/ustring.h>
#include <az/cfa2/fwd.h>
namespace az {
namespace cfa2 {

inline iv::core::UString GetFunctionPrototypeDeclaration(FunctionLiteral* fun) {
  assert(fun);
  iv::core::UString res;
  if (const iv::core::Maybe<Identifier> name = fun->name()) {
    res.append(name.Address()->value().begin(),
               name.Address()->value().end());
  } else {
    const iv::core::StringPiece piece("%anonymous");
    res.append(piece.begin(), piece.end());
  }
  res.push_back('(');
  Identifiers::const_iterator it = fun->params().begin();
  const Identifiers::const_iterator last = fun->params().end();
  if (it == last) {
    res.push_back(')');
    return res;
  }
  do {
    res.append((*it)->value().begin(), (*it)->value().end());
    ++it;
    if (it == last) {
      res.push_back(')');
      break;
    } else {
      res.push_back(',');
      res.push_back(' ');
    }
  } while (true);
  return res;
}

} }  // namespace az::cfa2
#endif  // _AZ_CFA2_TYPE_UTILS_H_
