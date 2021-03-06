// handling completion token
#ifndef AZ_COMPLETE_LEXER_H_
#define AZ_COMPLETE_LEXER_H_
#include <iv/lexer.h>
namespace az {

class CompleteLexer
  : public iv::core::Lexer<iv::core::UStringPiece, true, true> {
 public:
  typedef iv::core::Lexer<iv::core::UStringPiece, true, true> super_type;

  // complete lexer constructor
  CompleteLexer(const iv::core::UString& src, std::size_t len)
    : super_type(),
      original_(src),
      piece_(src.data(), len),
      before_complete_(true),
      completion_point_(false) {
    assert(original_.size() >= len);
    super_type::Initialize(&piece_);
  }

  // normal lexer constructor
  explicit CompleteLexer(const iv::core::UString& src)
    : super_type(),
      original_(src),
      piece_(src),
      before_complete_(false),
      completion_point_(false) {
    super_type::Initialize(&piece_);
  }

  template<typename LexType>
  typename iv::core::Token::Type Next(bool strict) {
    using iv::core::Token;
    const typename Token::Type token =
        super_type::template Next<LexType>(strict);
    if (token == Token::TK_EOS) {
      if (before_complete()) {
        before_complete_ = false;
        completion_point_ = true;
        piece_ = original_;
        Initialize(&piece_);
        return super_type::template Next<LexType>(strict);
      }
    }
    completion_point_ = false;
    return token;
  }

  bool before_complete() const {
    return before_complete_;
  }

  bool IsCompletionPoint() const {
    return completion_point_;
  }

 private:

  const iv::core::UString& original_;
  iv::core::UStringPiece piece_;
  bool before_complete_;
  bool completion_point_;
};

}  // namespace az
#endif  // AZ_COMPLETER_LEXER_H_
