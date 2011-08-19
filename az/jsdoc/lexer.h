#ifndef _AZ_JSDOC_LEXER_H_
#define _AZ_JSDOC_LEXER_H_
#include <iv/noncopyable.h>
#include <az/jsdoc/token.h>
#include <az/jsdoc/tag.h>
namespace az {
namespace jsdoc {

class Lexer : private iv::core::Noncopyable<Lexer> {
 public:
  explicit Lexer(const iv::core::UStringPiece& src)
    : source_(src),
      c_(-1),
      pos_(0),
      end_(src.size()) {
    Advance();
  }

  void Advance() {
    if (pos_ == end_) {
      c_ = -1;
    } else {
      c_ = source_[pos_++];
    }
  }

  Token::Type Next() {
    Token::Type token = Token::TK_NOT_FOUND;
    do {
      // skip to tag
      while (c_ >= 0 && c_ != '@') {
        Advance();
      }
      switch (c_) {
        case '@': {
          token = ScanTag();
          break;
        }

        default: {
          if (c_ < 0) {
            token = Token::TK_EOS;
          }
        }
      }
    } while (token == Token::TK_NOT_FOUND);
    return token;
  }

  Token::Type ScanTag() {
    assert(c_ == '@');
    buffer_.clear();
    buffer_.push_back('@');
    Advance();
    while (c_ >= 0 && iv::core::character::IsASCIIAlphanumeric(c_)) {
      buffer_.push_back(c_);
      Advance();
    }
    std::size_t index = 0;
    for (TagArray::const_iterator it = kKnownTags.begin(),
         last = kKnownTags.end(); it != last; ++it, ++index) {
      if (it->size() == buffer_.size() &&
          std::equal(it->begin(), it->end(), buffer_.begin())) {
        return static_cast<Token::Type>(index);
      }
    }
    // not known tag
    return Token::TK_TAG;
  }

 private:
  iv::core::UStringPiece source_;
  int c_;
  std::vector<char> buffer_;
  std::size_t pos_;
  std::size_t end_;
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_LEXER_H_
