#ifndef AZ_UTILITY_H_
#define AZ_UTILITY_H_
#include <algorithm>
#include <iv/stringpiece.h>
#include <iv/character.h>
#include <iv/ustring.h>
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

// original code is iv / lv5 JSON.stringify
inline iv::core::UString EscapedString(const iv::core::UStringPiece& piece) {
  static const char kHexDigits[17] = "0123456789ABCDEF";
  std::vector<uint16_t> builder;
  builder.push_back('"');
  for (iv::core::UStringPiece::const_iterator it = piece.begin(),
       last = piece.end(); it != last; ++it) {
    const uint16_t c = *it;
    if (c == '"' || c == '\\') {
      builder.push_back('\\');
      builder.push_back(c);
    } else if (c == '\b' ||
               c == '\f' ||
               c == '\n' ||
               c == '\r' ||
               c == '\t') {
      builder.push_back('\\');
      switch (c) {
        case '\b':
          builder.push_back('b');
          break;

        case '\f':
          builder.push_back('f');
          break;

        case '\n':
          builder.push_back('n');
          break;

        case '\r':
          builder.push_back('r');
          break;

        case '\t':
          builder.push_back('t');
          break;
      }
    } else if (c < ' ') {
      uint16_t val = c;
      std::array<char, 4> buf = { { '0', '0', '0', '0' } };
      builder.push_back('\\');
      builder.push_back('u');
      for (int i = 0; (i < 4) || val; i++) {
        buf[3 - i] = kHexDigits[val % 16];
        val /= 16;
      }
      builder.insert(builder.end(), buf.begin(), buf.end());
    } else {
      builder.push_back(c);
    }
  }
  builder.push_back('"');
  return iv::core::UString(builder.begin(), builder.end());
}

}  // namespace az
#endif  // AZ_UTILITY_H_
