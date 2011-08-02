#ifndef _AZ_CHARACTER_H_
#define _AZ_CHARACTER_H_
namespace az {
namespace character {

static const int CR = 0x000D;
static const int LF = 0x000A;

// CRLF
inline bool IsLineTerminatorCRLF(uint16_t first, uint16_t second) {
  return first == CR && second == LF;
}

} }  // namespace az::character
#endif  // _AZ_CHARACTER_H_
