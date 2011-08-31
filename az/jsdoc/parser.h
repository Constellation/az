#ifndef AZ_JSDOC_PARSER_H_
#define AZ_JSDOC_PARSER_H_
#include <iterator>
#include <iv/detail/memory.h>
#include <iv/noncopyable.h>
#include <az/utility.h>
#include <az/factory.h>
#include <az/debug_log.h>
#include <az/jsdoc/token.h>
#include <az/jsdoc/tag.h>
#include <az/jsdoc/info.h>
#include <az/jsdoc/type_ast.h>
#include <az/jsdoc/type_parser.h>
namespace az {
namespace jsdoc {

class Parser : private iv::core::Noncopyable<Parser> {
 public:
  explicit Parser(AstFactory* factory,
                  const iv::core::UStringPiece& src)
    : factory_(factory),
      source_(),
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
    if (IsEOS()) {
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

  bool IsEOS() const {
    return pos_ == end_;
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
    enum {
      BEFORE_STAR = 0,
      STAR,
      AFTER_STAR
    } mode = BEFORE_STAR;
    for (iv::core::UStringPiece::const_iterator it = target.begin(),
         last = target.end(); it != last; ++it) {
      if (mode == BEFORE_STAR) {
        if (iv::core::character::IsLineTerminator(*it)) {
          // waste
          *result++ = *it;
        } else if (iv::core::character::IsWhiteSpace(*it)) {
          // waste
        } else if (*it == '*') {
          // waste and change mode 1
          mode = STAR;
        } else {
          // get and change mode 2
          mode = AFTER_STAR;
          *result++ = *it;
        }
      } else if (mode == STAR) {
        if (!iv::core::character::IsWhiteSpace(*it)) {
          *result++ = *it;
        }
        mode = AFTER_STAR;
      } else {
        assert(mode == AFTER_STAR);
        *result++ = *it;
        if (iv::core::character::IsLineTerminator(*it)) {
          mode = BEFORE_STAR;
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

    if (title_.empty()) {
      return std::shared_ptr<Tag>();
    }
    tag->set_title(title_);

    if (IsTypeRequiredToken()) {
      if (type_.empty()) {
        return std::shared_ptr<Tag>();
      }
      TypeParser parser(factory_, iv::core::UStringPiece(type_.data(), type_.size()));
      if (TypeExpression* expr =
          (token_ == Token::TK_PARAM) ?
          parser.ParseParamType() : parser.ParseType()) {
        tag->set_type(expr);
      } else {
        return std::shared_ptr<Tag>();
      }
    }

    if (IsNameRequiredToken()) {
      if (name_.empty()) {
        return std::shared_ptr<Tag>();
      }
      if (!iv::core::character::IsIdentifierStart(name_[0])) {
        return std::shared_ptr<Tag>();
      }
      for (std::vector<uint16_t>::const_iterator it = name_.begin() + 1,
           last = name_.end(); it != last; ++it) {
        if (!iv::core::character::IsIdentifierPart(*it)) {
          return std::shared_ptr<Tag>();
        }
      }
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
      if (IsEqual(*it, title_)) {
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
    // type expression may have nest brace, such as,
    // { { ok: string } }
    //
    // therefore, scanning type expression with balancing braces.
    type_.clear();
    std::vector<uint16_t>::const_iterator it = content.begin();
    for (const std::vector<uint16_t>::const_iterator last = content.end();
         it != last; ++it) {
      if (iv::core::character::IsWhiteSpace(*it)) {
        // through it
      } else if (*it == '{') {
        // left brace found
        ++it;
        break;
      } else {
        // type specifier is not found
        type_.clear();
        return 0;
      }
    }
    // type expression { is found
    std::size_t brace = 0;
    for (const std::vector<uint16_t>::const_iterator last = content.end();
         it != last; ++it) {
      if (iv::core::character::IsLineTerminator(*it)) {
        // failed
        break;
      } else if (*it == '}') {
        // right brace found
        if (brace == 0) {
          // end
          return std::distance(content.begin(), ++it);
        } else {
          --brace;
        }
      } else if (*it == '{') {
        ++brace;
      }
      type_.push_back(*it);
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

  AstFactory* factory_;
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
#endif  // AZ_JSDOC_PARSER_H_
