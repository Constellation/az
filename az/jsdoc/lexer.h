#ifndef _AZ_JSDOC_LEXER_H_
#define _AZ_JSDOC_LEXER_H_
#include <iterator>
#include <iv/noncopyable.h>
#include <az/debug_log.h>
#include <az/jsdoc/token.h>
#include <az/jsdoc/tag.h>
namespace az {
namespace jsdoc {

class Lexer : private iv::core::Noncopyable<Lexer> {
 public:
  explicit Lexer(const iv::core::UStringPiece& src)
    : source_(),
      c_(-1),
      title_(),
      content_(),
      pos_(0),
      end_() {
    source_.reserve(src.size() - 5);
    Lexer::UnwrapComment(src, std::back_inserter(source_));
    end_ = source_.size();
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
    ScanTitle();
    ScanContent();
    return token_;
  }

  void ScanTitle() {
    assert(c_ == '@');
    title_.clear();
    // parse title string
    title_.push_back('@');
    Advance();
    while (c_ >= 0 && iv::core::character::IsASCIIAlphanumeric(c_)) {
      title_.push_back(c_);
      Advance();
    }
    assert(c_ < 0 || !iv::core::character::IsASCIIAlphanumeric(c_));

    std::size_t index = 0;
    token_ = Token::TK_TAG;
    for (TagArray::const_iterator it = kKnownTags.begin(),
         last = kKnownTags.end(); it != last; ++it, ++index) {
      if (it->size() == title_.size() &&
          std::equal(it->begin(), it->end(), title_.begin())) {
        token_ = static_cast<Token::Type>(index);
        break;
      }
    }
  }

  void ScanContent() {
    content_.clear();
    bool waiting_next_tag = false;
    while (c_ >= 0) {
      if (iv::core::character::IsLineTerminator(c_)) {
        // Line Terminator found
        waiting_next_tag = true;
      } else {
        if (waiting_next_tag) {
          if (c_ == '@') {
            // next tag found
            break;
          }
          if (!iv::core::character::IsWhiteSpace(c_)) {
            waiting_next_tag = false;
          }
        }
      }
      content_.push_back(c_);
      Advance();
    }
  }

  template<typename OutputIter>
  static void UnwrapComment(const iv::core::UStringPiece& src,
                            OutputIter result) {
    // JSDoc comment is following form
    //   /**
    //    * ........
    //    */
    // so, remove /**, */ and *.
    assert(src.size() >= 5);
    assert(src[0] == '/' && src[1] == '*' && src[2] == '*');
    assert(src[src.size() - 1] == '/' && src[src.size() - 2] == '*');
    // remove pre/postfix slash star
    const iv::core::UStringPiece target(src.data() + 3, src.size() - 5);

    // remove line prefix slash star
    // mode indicates
    //      * 
    //   0 |1| 2
    int mode = 0;
    for (iv::core::UStringPiece::const_iterator it = target.begin(),
         last = target.end(); it != last; ++it) {
      if (mode == 0) {
        if (iv::core::character::IsLineTerminator(*it)) {
          // waste
          *result++ = *it;
        } else if (iv::core::character::IsWhiteSpace(*it)) {
          // waste
        } else if (*it == '*') {
          // waste and change mode 1
          mode = 1;
        } else {
          // get and change mode 2
          mode = 2;
          *result++ = *it;
        }
      } else if (mode == 1) {
        if (!iv::core::character::IsWhiteSpace(*it)) {
          *result++ = *it;
        }
        mode = 2;
      } else {
        assert(mode == 2);
        *result++ = *it;
        if (iv::core::character::IsLineTerminator(*it)) {
          mode = 0;
        }
      }
    }
  }

 private:
  std::vector<uint16_t> source_;
  int c_;
  Token::Type token_;
  std::vector<char> title_;
  std::vector<uint16_t> content_;
  std::size_t pos_;
  std::size_t end_;
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_LEXER_H_
