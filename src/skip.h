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

  void SkipUntilSemicolonOrLineTerminator(std::size_t end) {
    // TODO(Constellation) skip if comment like found
    std::size_t line_number = lexer_->line_number();
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
