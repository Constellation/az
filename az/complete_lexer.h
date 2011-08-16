// handling completion token
#ifndef _AZ_COMPLETE_LEXER_H_
#define _AZ_COMPLETE_LEXER_H_
#include <iv/lexer.h>
namespace az {

class CompleteLexer : public iv::core::Lexer<iv::core::UStringPiece, true, true> {
 public:
  typedef iv::core::Lexer<iv::core::UStringPiece, true, true> super_type;

  CompleteLexer(const iv::core::UString& src, std::size_t len)
    : super_type(),
      original_(src),
      piece_(src.data(), len),
      previous_of_complete_(true),
      completion_point_(false) {
    assert(original_.size() >= len);
    super_type::Initialize(&piece_);
  }

  CompleteLexer(const iv::core::UString& src)
    : super_type(),
      original_(src),
      piece_(src),
      previous_of_complete_(false),
      completion_point_(false) {
    super_type::Initialize(&piece_);
  }

  template<typename LexType>
  typename iv::core::Token::Type Next(bool strict) {
    using iv::core::Token;
    const typename Token::Type token =
        super_type::template Next<LexType>(strict);
    if (token == Token::TK_EOS) {
      if (previous_of_complete()) {
        previous_of_complete_ = false;
        completion_point_ = true;
        piece_ = original_;
        Initialize(&piece_);
        return super_type::template Next<LexType>(strict);
      }
    }
    completion_point_ = false;
    return token;
  }

  bool previous_of_complete() const {
    return previous_of_complete_;
  }

  bool IsCompletionPoint() const {
    return completion_point_;
  }

 private:

  const iv::core::UString& original_;
  iv::core::UStringPiece piece_;
  bool previous_of_complete_;
  bool completion_point_;
};

}  // namespace az
#endif  //_AZ_COMPLETER_LEXER_H_
