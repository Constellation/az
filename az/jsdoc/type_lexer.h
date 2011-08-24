// Type Lexer lex TypeExpression in JSDoc.
#ifndef _AZ_JSDOC_TYPE_LEXER_H_
#define _AZ_JSDOC_TYPE_LEXER_H_
#include <iv/noncopyable.h>
#include <iv/ustringpiece.h>
#include <iv/character.h>
#include <az/jsdoc/fwd.h>
#include <az/jsdoc/type_token.h>
namespace az {
namespace jsdoc {

class TypeLexer : private iv::core::Noncopyable<TypeLexer> {
 public:
  explicit TypeLexer(const iv::core::UStringPiece& source)
    : source_(source),
      pos_(0),
      end_(source_.size()),
      c_(-1),
      buffer_() {
    Advance();
  }

  TypeToken::Type Next() {
    while (c_ >= 0 && iv::core::character::IsWhiteSpace(c_)) {
      // white space and line terminator
      Advance();
    }
    switch (c_) {
      case ':':
        Advance();
        return TypeToken::TK_COLON;

      case ',':
        Advance();
        return TypeToken::TK_COMMA;

      case '(':
        Advance();
        return TypeToken::TK_LPAREN;

      case ')':
        Advance();
        return TypeToken::TK_RPAREN;

      case '[':
        Advance();
        return TypeToken::TK_LBRACK;

      case ']':
        Advance();
        return TypeToken::TK_RBRACK;

      case '{':
        Advance();
        return TypeToken::TK_LBRACE;

      case '}':
        Advance();
        return TypeToken::TK_RBRACE;

      case '.':
        Advance();
        if (c_ == '<') {
          Advance();
          return TypeToken::TK_DOT_LT;
        } else if (c_ == '.') {
          Advance();
          if (c_ == '.') {
            Advance();
            return TypeToken::TK_REST;
          }
          PushBack();
        }
        return TypeToken::TK_DOT;

      case '>':
        Advance();
        return TypeToken::TK_GT;

      case '*':
        Advance();
        return TypeToken::TK_STAR;

      case '|':
        Advance();
        return TypeToken::TK_PIPE;

      case '?':
        Advance();
        return TypeToken::TK_QUESTION;

      case '!':
        Advance();
        return TypeToken::TK_BANG;

      case '=':
        Advance();
        return TypeToken::TK_EQUAL;

      default: {
        if (c_ < 0) {
          // EOS
          return TypeToken::TK_EOS;
        } else if (TypeLexer::IsTypeName(c_)) {
          // type string permits following case,
          //
          // namespace.module.MyClass
          //
          // this reduced 1 token TK_NAME
          return ScanTypeName();
        } else {
          return TypeToken::TK_ILLEGAL;
        }
      }
    }
  }

  const std::vector<uint16_t>& Buffer() const {
    return buffer_;
  }

 private:
  void Advance() {
    if (pos_ == end_) {
      c_ = -1;
    } else {
      c_ = source_[pos_++];
    }
  }

  void PushBack() {
    if (pos_ < 2) {
      c_ = -1;
    } else {
      c_ = source_[pos_-2];
      --pos_;
    }
  }

  static bool IsTypeName(int ch) {
    return
        ch >= 0   && ch != '>' && ch != '<' && ch != '(' && ch != ')' &&
        ch != '{' && ch != '}' && ch != '[' && ch != ']' && ch != ',' &&
        ch != ':' && ch != '*' && ch != '|' && ch != '?' && ch != '!' &&
        ch != '=' && !iv::core::character::IsWhiteSpace(ch);
  }

  TypeToken::Type ScanTypeName() {
    buffer_.clear();
    while (TypeLexer::IsTypeName(c_)) {
      if (c_ == '.') {
        Advance();
        if (c_ == '<') {
          PushBack();
          break;
        }
        Record('.');
      } else {
        Record(c_);
        Advance();
      }
    }
    return TypeToken::TK_NAME;
  }

  inline void Record(const int ch) {
    buffer_.push_back(ch);
  }

  iv::core::UStringPiece source_;
  std::size_t pos_;
  std::size_t end_;
  int c_;
  std::vector<uint16_t> buffer_;
};

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TYPE_LEXER_H_
