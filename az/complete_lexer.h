// handling completion token
#ifndef _AZ_COMPLETE_LEXER_H_
#define _AZ_COMPLETE_LEXER_H_
#include <iv/lexer.h>
namespace az {

class CompleteLexer : public iv::core::Lexer<iv::core::UStringPiece, true, true> {
 public:
  typedef iv::core::Lexer<iv::core::UStringPiece, true, true> super_type;

  static const int kCompleteToken = iv::core::Token::TK_NOT_FOUND;

  explicit CompleteLexer(const iv::core::UString& src, std::size_t position)
    : super_type(),
      original_(src),
      piece_(src.data(), position),
      previous_of_complete_(true) {
    super_type::Initialize(piece_);
  }

  template<typename LexType>
  typename iv::core::Token::Type Next(bool strict) {
    using iv::core::Token;
    const typename Token::Type token =
        super_type::template Next<LexType>(strict);
    if (token == Token::TK_EOS) {
      if (previous_of_complete()) {
        previous_of_complete_ = false;
        Initialize(original_);
        return Token::TK_NOT_FOUND;  // TK_NOT_FOUND is used for completion
      }
    }
    return token;
  }

  bool previous_of_complete() const {
    return previous_of_complete_;
  }

 private:

  const iv::core::UString& original_;
  iv::core::UStringPiece piece_;
  bool previous_of_complete_;
};

}  // namespace az
#endif  //_AZ_COMPLETER_LEXER_H_
