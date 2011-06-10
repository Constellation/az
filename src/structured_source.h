#ifndef _AZ_STRUCTURED_SOURCE_H_
#define _AZ_STRUCTURED_SOURCE_H_
#include <cstddef>
#include <cassert>
#include <vector>
#include <algorithm>
#include <iv/detail/cstdint.h>
#include <iv/ustringpiece.h>
#include <iv/character.h>
namespace az {

class StructuredSource : private iv::core::Noncopyable<StructuredSource> {
 public:
  typedef std::pair<std::size_t, std::size_t> Line;  // start position and length
  typedef std::vector<Line> Lines;

  explicit StructuredSource(const iv::core::UStringPiece& str)
    : source_() {
    std::size_t pos = 0;
    std::size_t line_start_pos = pos;
    iv::core::UStringPiece::const_iterator line_start_it = str.begin();
    for (iv::core::UStringPiece::const_iterator it = str.begin(),
         last = str.end(); it != last; ++it, ++pos) {
      if (iv::core::character::IsLineTerminator(*it)) {
        iv::core::UStringPiece::const_iterator next_it = it;
        ++next_it;
        if (next_it != last) {
          if (*it + *next_it == '\n' + '\r') {
            it = next_it;
            ++pos;
          }
        }
        source_.push_back(std::make_pair(line_start_pos, std::distance(line_start_it, it + 1)));
        line_start_pos = pos + 1;
        line_start_it = it + 1;
      }
    }
    if (line_start_it != str.end()) {
      source_.push_back(std::make_pair(line_start_pos, std::distance(line_start_it, str.end())));
    }
  }

  const Lines& GetLines() const {
    return source_;
  }

  struct LowerBoundFinder {
    bool operator()(const Line& lhs, std::size_t rhs) const {
      return lhs.first < rhs;
    }

    bool operator()(const Line& lhs, const Line& rhs) const {
      return lhs.first < rhs.first;
    }

    bool operator()(std::size_t lhs, const Line& rhs) const {
      return lhs < rhs.first;
    }
  };

  std::pair<std::size_t, std::size_t> GetLineAndColumn(std::size_t pos) const {
    if (source_.empty()) {
      return std::make_pair(1, pos + 1);
    }
    // Lines first is always 0
    Lines::const_iterator it =
        std::upper_bound(source_.begin(), source_.end(), pos, LowerBoundFinder());
    std::advance(it, -1);
    assert(it != source_.end());
    return std::make_pair(
        std::distance(source_.begin(), it) + 1,
        (pos + 1) - it->first);
  }

 private:
  Lines source_;
};

}  // namespace az
#endif  // _AZ_STRUCTURED_SOURCE_H_
