#ifndef _AZ_JSDOC_PARSER_H_
#define _AZ_JSDOC_PARSER_H_
#include <iterator>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/debug_log.h>
#include <az/jsdoc/token.h>
#include <az/jsdoc/tag.h>
#include <az/jsdoc/info.h>
namespace az {
namespace jsdoc {

class Parser : private iv::core::Noncopyable<Parser> {
 public:
  explicit Parser(const iv::core::UStringPiece& src)
    : source_(),
      c_(-1),
      content_(),
      title_(),
      type_(),
      name_(),
      desc_(),
      pos_(0),
      end_() {
    source_.reserve(src.size() - 5);
    Parser::UnwrapComment(src, std::back_inserter(source_));
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

  std::shared_ptr<Tag> Next() {
    // skip to tag
    while (c_ >= 0 && c_ != '@') {
      Advance();
    }
    assert(c_ < 0 || c_ == '@');
    if (c_ == '@') {
      return ScanTag();
    } else {
      return std::shared_ptr<Tag>();
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

  Token::Type token() const {
    return token_;
  }

  const std::vector<uint16_t>& source() const {
    return source_;
  }

  const std::vector<uint16_t>& content() const {
    return content_;
  }

  const std::vector<uint16_t>& title() const {
    return title_;
  }

  const std::vector<uint16_t>& type() const {
    return type_;
  }

  const std::vector<uint16_t>& name() const {
    return name_;
  }

  const std::vector<uint16_t>& desc() const {
    return desc_;
  }

 private:
  std::shared_ptr<Tag> ScanTag() {
    assert(c_ == '@');
    ScanTitle();
    ScanContent();
    std::shared_ptr<Tag> tag(new Tag(token_));
    tag->set_title(title_);
    if (!type_.empty()) {
      tag->set_type(type_);
    }
    if (IsNameRequiredToken()) {
      tag->set_name(name_);
    }
    tag->set_description(desc_);
    return tag;
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
    std::size_t i = 0;
    if (IsTypeRequiredToken()) {
      i = ScanType(content_);
    }
    if (IsNameRequiredToken()) {
      i = ScanName(content_, i);
    }
    ScanDesc(content_, i);
  }

  std::size_t ScanType(const std::vector<uint16_t>& content) {
    type_.clear();
    bool in_type_brace = false;
    for (std::vector<uint16_t>::const_iterator it = content.begin(),
         last = content.end(); it != last; ++it) {
      if (!in_type_brace) {
        if (iv::core::character::IsWhiteSpace(*it)) {
          // through it
        } else if (*it == '{') {
          // left brace found
          in_type_brace = true;
        } else {
          // type specifier is not found
          break;
        }
      } else {
        if (iv::core::character::IsLineTerminator(*it)) {
          break;
        } else if (*it == '}') {
          // right brace found
          return std::distance(content.begin(), ++it);
        } else {
          type_.push_back(*it);
        }
      }
    }
    type_.clear();
    return 0; // type speficier parsing is failed
  }

  std::size_t ScanName(const std::vector<uint16_t>& content, std::size_t i) {
    bool in_name = false;
    name_.clear();
    for (std::vector<uint16_t>::const_iterator it = content.begin() + i,
         last = content.end(); it != last; ++it) {
      if (!in_name) {
        if (iv::core::character::IsWhiteSpace(*it) ||
            iv::core::character::IsLineTerminator(*it)) {
          // through it
        } else {
          in_name = true;
          name_.push_back(*it);
        }
      } else {
        if (iv::core::character::IsWhiteSpace(*it) ||
            iv::core::character::IsLineTerminator(*it)) {
          return std::distance(content.begin(), ++it);
        } else {
          name_.push_back(*it);
        }
      }
    }
    // EOS found
    return content.size();
  }

  void ScanDesc(const std::vector<uint16_t>& content, std::size_t i) {
    desc_.assign(content.begin() + i, content.end());
  }

  bool IsTypeRequiredToken() const {
    return
        token_ == Token::TK_DEFINE ||
        token_ == Token::TK_ENUM ||
        token_ == Token::TK_EXTENDS ||
        token_ == Token::TK_IMPLEMENTS ||
        token_ == Token::TK_PARAM ||
        token_ == Token::TK_RETURN ||
        token_ == Token::TK_THIS ||
        token_ == Token::TK_TYPE ||
        token_ == Token::TK_TYPEDEF;
  }

  bool IsNameRequiredToken() const {
    return token_ == Token::TK_PARAM;
  }

  std::vector<uint16_t> source_;
  int c_;
  Token::Type token_;
  std::vector<uint16_t> content_;
  std::vector<uint16_t> title_;
  std::vector<uint16_t> type_;
  std::vector<uint16_t> name_;
  std::vector<uint16_t> desc_;
  std::size_t pos_;
  std::size_t end_;
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_PARSER_H_
