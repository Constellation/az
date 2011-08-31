// Type Lexer lex TypeExpression in JSDoc.
#ifndef AZ_JSDOC_TYPE_LEXER_H_
#define AZ_JSDOC_TYPE_LEXER_H_
#include <iv/noncopyable.h>
#include <iv/ustringpiece.h>
#include <iv/character.h>
#include <az/utility.h>
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
      buffer_(),
      buffer8_(),
      numeric_() {
    Advance();
  }

  TypeToken::Type Next() {
    while (c_ >= 0 && iv::core::character::IsWhiteSpace(c_)) {
      // white space and line terminator
      Advance();
    }
    switch (c_) {
      case '"':
        return ScanString();

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
        } else if (iv::core::character::IsDecimalDigit(c_)) {
          return ScanNumber();
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

  bool IsNullLiteral() const {
    return IsEqual(buffer_, "null");
  }

  bool IsUndefinedLiteral() const {
    return IsEqual(buffer_, "undefined");
  }

  bool IsFunction() const {
    return IsEqual(buffer_, "function");
  }

  bool IsThisLiteral() const {
    return IsEqual(buffer_, "this");
  }

  bool IsVoidLiteral() const {
    return IsEqual(buffer_, "void");
  }

  bool IsNewLiteral() const {
    return IsEqual(buffer_, "new");
  }

  double Numeric() const {
    return numeric_;
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
        Record();
        Advance();
      }
    }
    return TypeToken::TK_NAME;
  }

  // allow JSONNumber
  TypeToken::Type ScanNumber() {
    buffer8_.clear();
    if (c_ == '0') {
      Record8();
      Advance();
    } else {
      ScanDecimalDigits();
    }
    if (c_ == '.') {
      Record8();
      Advance();
      if (c_ < 0 ||
          !iv::core::character::IsDecimalDigit(c_)) {
        return TypeToken::TK_ILLEGAL;
      }
      ScanDecimalDigits();
    }

    // exponent part
    if (c_ == 'e' || c_ == 'E') {
      Record8();
      Advance();
      if (c_ == '+' || c_ == '-') {
        Record8();
        Advance();
      }
      // more than 1 decimal digit required
      if (c_ < 0 ||
          !iv::core::character::IsDecimalDigit(c_)) {
        return TypeToken::TK_ILLEGAL;
      }
      ScanDecimalDigits();
    }
    buffer8_.push_back('\0');
    numeric_ = std::atof(buffer8_.data());
    return TypeToken::TK_NUMBER;
  }

  void ScanDecimalDigits() {
    while (0 <= c_ && iv::core::character::IsDecimalDigit(c_)) {
      Record8();
      Advance();
    }
  }

  // allow JSONString
  TypeToken::Type ScanString() {
    assert(c_ == '"');
    buffer_.clear();
    Advance();
    while (c_ != '"' && c_ >= 0) {
      if (c_ == '\\') {
        Advance();
        // escape sequence
        if (c_ < 0) {
          return TypeToken::TK_ILLEGAL;
        }
        if (!ScanEscape()) {
          return TypeToken::TK_ILLEGAL;
        }
      } else if (c_ > 0x001F &&
                 !iv::core::character::IsLineTerminator(c_)) {
        Record();
        Advance();
      } else {
        return TypeToken::TK_ILLEGAL;
      }
    }
    if (c_ != '"') {
      // not closed
      return TypeToken::TK_ILLEGAL;
    }
    Advance();
    return TypeToken::TK_STRING;
  }

  bool ScanEscape() {
    switch (c_) {
      case '"' :
      case '/':
      case '\\':
        Record();
        Advance();
        break;
      case 'b' :
        Record('\b');
        Advance();
        break;
      case 'f' :
        Record('\f');
        Advance();
        break;
      case 'n' :
        Record('\n');
        Advance();
        break;
      case 'r' :
        Record('\r');
        Advance();
        break;
      case 't' :
        Record('\t');
        Advance();
        break;
      case 'u' : {
        Advance();
        uint16_t uc = '\0';
        for (int i = 0; i < 4; ++i) {
          const int d = iv::core::HexValue(c_);
          if (d < 0) {
            return false;
          }
          uc = uc * 16 + d;
          Advance();
        }
        Record(uc);
        break;
      }
      default:
        // in JSON syntax, NonEscapeCharacter not found
        return false;
    }
    return true;
  }

  inline void Record(int ch) {
    buffer_.push_back(ch);
  }

  inline void Record() {
    buffer_.push_back(c_);
  }

  inline void Record8(int ch) {
    buffer8_.push_back(ch);
  }

  inline void Record8() {
    buffer8_.push_back(c_);
  }

  iv::core::UStringPiece source_;
  std::size_t pos_;
  std::size_t end_;
  int c_;
  std::vector<uint16_t> buffer_;
  std::vector<char> buffer8_;
  double numeric_;
};

} }  // namespace az::jsdoc
#endif  // AZ_JSDOC_TYPE_LEXER_H_
