#ifndef AZ_UTILITY_H_
#define AZ_UTILITY_H_
#include <algorithm>
#include <iv/stringpiece.h>
#include <iv/character.h>
namespace az {

// ignore case check
// target is ASCII only
template<typename Container1, typename Container2>
inline bool IsEqualIgnoreCase(const Container1& piece, const Container2& target) {
  if (piece.size() != target.size()) {
    return false;
  }
  iv::core::StringPiece::const_iterator current = target.begin();
  for (typename Container1::const_iterator it = piece.begin(),
       last = piece.end(); it != last; ++it, ++current) {
    if (iv::core::character::ToLowerCase(*it) !=
        iv::core::character::ToLowerCase(*current)) {
      return false;
    }
  }
  return true;
}

template<typename Container>
inline bool IsEqualIgnoreCase(const Container& piece, const char* target) {
  return IsEqualIgnoreCase(piece, iv::core::StringPiece(target));
}

template<typename Container1, typename Container2>
inline bool IsEqual(const Container1& piece, const Container2& target) {
  if (piece.size() != target.size()) {
    return false;
  }
  return std::equal(piece.begin(), piece.end(), target.begin());
}

template<typename Container>
inline bool IsEqual(const Container& piece, const char* target) {
  return IsEqual(piece, iv::core::StringPiece(target));
}


}  // namespace az
#endif  // AZ_UTILITY_H_
