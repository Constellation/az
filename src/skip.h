#ifndef _AZ_SKIP_H_
#define _AZ_SKIP_H_
namespace az {

// Skip class skips heuristic skip sources
template<typename Lexer>
class BasicSkip {
 public:
  explicit BasicSkip(Lexer* lexer)
    : lexer_(lexer),
      source_(lexer->source()) {
  }

  void SkipUntilSemicolonOrLineTerminator(std::size_t end, std::size_t line_number) {
    // TODO(Constellation) skip if comment like found
    for (std::size_t i = end, len = source_.size(); i < len; ++i) {
      if (source_[i] == ';') {
        lexer_->SkipTo(i + 1, line_number, false);
        return;
      } else if (iv::core::character::IsLineTerminator(source_[i])) {
        if (i + 1 < len &&
            source_[i] + source_[i + 1] == '\r' + '\n') {
          ++i;
        }
        ++line_number;
        lexer_->SkipTo(i + 1, line_number, true);
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
                  if (i + 1 < len &&
                      source_[i] + source_[i + 1] == '\r' + '\n') {
                    ++i;
                  }
                  ++line_number;
                }
              } else {
                break;
              }
            }
          } else if (iv::core::character::IsLineTerminator(ch)) {
            if (i + 1 < len &&
                ch + source_[i + 1] == '\r' + '\n') {
              ++i;
            }
            ++line_number;
            lexer_->SkipTo(i + 1, line_number, true);
            return;
          } else {
            // string end found
            break;
          }
        }
      }
    }
    // EOS found...
    lexer_->SkipTo(source_.size(), line_number, false);
  }

 private:
  Lexer* lexer_;
  const typename Lexer::source_type& source_;
};

}  // namespace az
#endif  // _AZ_SKIP_H_
