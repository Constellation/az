#ifndef _AZ_SKIP_H_
#define _AZ_SKIP_H_
#include "character.h"
#include "structured_source.h"
namespace az {

// Skip class skips heuristic skip sources
template<typename Lexer>
class BasicSkip {
 public:
  explicit BasicSkip(Lexer* lexer, const StructuredSource& structured)
    : lexer_(lexer),
      source_(lexer->source()),
      structured_(structured) {
  }

  void SkipUntil(std::size_t end, const iv::core::UStringPiece& str) {
    SkipUntilImpl(end, str);
  }

  void SkipUntil(std::size_t end, const iv::core::StringPiece& str) {
    SkipUntilImpl(end, str);
  }

  void SkipUntilSemicolonOrLineTerminator(std::size_t end) {
    SkipUntil(end, ";");
  }

 private:
  template<typename Piece>
  void SkipUntilImpl(std::size_t end, const Piece& str) {
    // TODO(Constellation) refactoring this method
    for (std::size_t i = end, len = source_.size(); i < len; ++i) {
      if (iv::core::character::IsLineTerminator(source_[i])) {
        if (i + 1 < len && character::IsLineTerminatorCRLF(source_[i], source_[i + 1])) {
          ++i;
        }
        lexer_->SkipTo(i + 1, structured_.GetLineAndColumn(i + 1).first, true);
        return;
      } else if (source_[i] == '"' || source_[i] == '\'') {
        // skip string
        // Lexer#ScanString scans String Format strictly,
        // but, in recovery phase skips String loosely.
        const uint16_t quote = source_[i];
        ++i;
        for (; i < len; ++i) {
          const uint16_t ch = source_[i];
          if (ch != quote && !iv::core::character::IsLineTerminator(ch)) {
            if (ch == '\\') {
              // loosely scanning in escape phase
              ++i;
              if (i < len) {
                if (iv::core::character::IsLineTerminator(source_[i])) {
                  if (i + 1 < len && character::IsLineTerminatorCRLF(source_[i], source_[i + 1])) {
                    ++i;
                  }
                }
              } else {
                break;
              }
            }
          } else if (iv::core::character::IsLineTerminator(ch)) {
            if (i + 1 < len && character::IsLineTerminatorCRLF(source_[i], source_[i + 1])) {
              ++i;
            }
            lexer_->SkipTo(i + 1, structured_.GetLineAndColumn(i + 1).first, true);
            return;
          } else {
            // string end found
            break;
          }
        }
      } else if (source_[i] == '/') {
        // comment like found
        if (i + 1 < len) {
          if (source_[i + 1] == '/') {
            // single line comment found
            // skip to LineTerminator
            ++i;
            for (; i < len; ++i) {
              if (iv::core::character::IsLineTerminator(source_[i])) {
                if (i + 1 < len && character::IsLineTerminatorCRLF(source_[i], source_[i + 1])) {
                  ++i;
                }
                lexer_->SkipTo(i + 1, structured_.GetLineAndColumn(i + 1).first, true);
                return;
              }
            }
          } else if (source_[i + 1] == '*') {
            // multi line comment found
            bool line_terminator = false;
            ++i;
            for (; i < len; ++i) {
              if (source_[i] == '*' && i + 1 < len && source_[i + 1] == '/') {
                // end of multi line comment
                ++i;
                if (line_terminator) {
                  lexer_->SkipTo(i + 1, structured_.GetLineAndColumn(i + 1).first, true);
                  return;
                } else {
                  break;
                }
              } else if (iv::core::character::IsLineTerminator(source_[i])) {
                if (i + 1 < len && character::IsLineTerminatorCRLF(source_[i], source_[i + 1])) {
                  ++i;
                }
                line_terminator = true;
              }
            }
          }
        }
      } else if (str.find(source_[i]) != iv::core::UStringPiece::npos) {
        // target character is found
        lexer_->SkipTo(i + 1, structured_.GetLineAndColumn(i + 1).first, false);
        return;
      }
    }
    // EOS found...
    lexer_->SkipTo(source_.size(), structured_.GetLineAndColumn(source_.size()).first, true);
  }




  Lexer* lexer_;
  const typename Lexer::source_type& source_;
  const StructuredSource& structured_;
};

}  // namespace az
#endif  // _AZ_SKIP_H_
