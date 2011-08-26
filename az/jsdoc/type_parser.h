// Type Parser parse TypeExpression in JSDoc.
//
// original TypeExpression parser code is closure-compiler
//
#ifndef _AZ_JSDOC_TYPE_PARSER_H_
#define _AZ_JSDOC_TYPE_PARSER_H_
#include <iv/noncopyable.h>
#include <iv/ustringpiece.h>
#include <iv/character.h>
#include <az/jsdoc/fwd.h>
#include <az/jsdoc/type_token.h>
#include <az/jsdoc/type_lexer.h>
#include <az/jsdoc/type_ast.h>
namespace az {
namespace jsdoc {

#define IS(token)\
  do {\
    if (token_ != token) {\
      *res = false;\
      return NULL;\
    }\
  } while (0)

#define EXPECT(token)\
  do {\
    if (token_ != token) {\
      *res = false;\
      return NULL;\
    }\
    Next();\
  } while (0)

#define UNEXPECT(token)\
  do {\
    *res = false;\
    return NULL;\
  } while (0)

#define CHECK  res);\
  if (!*res) {\
    return NULL;\
  }\
  ((void)0
#define DUMMY )  // to make indentation work
#undef DUMMY

class TypeParser : private iv::core::Noncopyable<Parser> {
 public:
  explicit TypeParser(const iv::core::UStringPiece& source)
    : lexer_(source) {
  }

  TypeExpression* ParseParamType() {
    // top level
    bool ok = true;
    Next();
    return ParseTypeExpression(&ok);
  }

  TypeExpression* ParseType() {
    // top level
    return NULL;
  }

 private:

  TypeExpression* ParseTypeExpression(bool* res) {
    // TypeExpression := BasicTypeExpression
    //     | '?' BasicTypeExpression
    //     | '!' BasicTypeExpression
    //     | BasicTypeExpression '?'
    //     | BasicTypeExpression '!'
    //     | '?'
    if (token_ == TypeToken::TK_QUESTION) {
      Next();
      if (token_ == TypeToken::TK_COMMA ||
          token_ == TypeToken::TK_EQUAL ||
          token_ == TypeToken::TK_RBRACE ||
          token_ == TypeToken::TK_RPAREN ||
          token_ == TypeToken::TK_PIPE) {
        // '?'
        return NewQuestionLiteral();
      }
      TypeExpression* expr = ParseBasicTypeExpression(CHECK);
      return NewPrefixQuestionExpression(expr);
    } else if (token_ == TypeToken::TK_BANG) {
      Next();
      TypeExpression* expr = ParseBasicTypeExpression(CHECK);
      return NewPrefixBangExpression(expr);
    } else {
      TypeExpression* expr = ParseBasicTypeExpression(CHECK);
      // check postfix
      if (token_ == TypeToken::TK_BANG) {
        Next();
        return NewPostfixBangExpression(expr);
      } else if (token_ == TypeToken::TK_QUESTION) {
        Next();
        return NewPostfixQuestionExpression(expr);
      } else {
        return expr;
      }
    }
  }

  TypeExpression* ParseBasicTypeExpression(bool* res) {
    // BasicTypeExpression := '*' | 'null' | 'undefined' | TypeName
    //     | FunctionType | UnionType | RecordType | ArrayType
    if (token_ == TypeToken::TK_STAR) {
      Next();
      return NewStarLiteral();
    } else if (token_ == TypeToken::TK_LPAREN) {
      return ParseUnionType(res);
    } else if (token_ == TypeToken::TK_LBRACK) {
      return ParseArrayType(res);
    } else if (token_ == TypeToken::TK_LBRACE) {
      return ParseRecordType(res);
    } else if (token_ == TypeToken::TK_NAME) {
      if (lexer_.IsNullLiteral()) {
        Next();
        return NewNullLiteral();
      } else if (lexer_.IsUndefinedLiteral()) {
        Next();
        return NewUndefinedLiteral();
      } else if (lexer_.IsFunction()) {
        return ParseFunctionType(res);
      }
      return ParseTypeName(res);
    } else {
      UNEXPECT(token_);
    }
  }

  UnionType* ParseUnionType(bool* res) {
    // UnionType := '(' TypeUnionList ')'
    // TypeUnionList := TypeExpression | TypeExpression '|' TypeUnionList
    assert(token_ == TypeToken::TK_LBRACE);
    Next();
    TypeExpressions* vec = NewTypeExpressions();
    while (true) {
      TypeExpression* expr = ParseTypeExpression(CHECK);
      vec->push_back(expr);
      if (token_ == TK_PIPE) {
        Next();
      } else {
        break;
      }
    }
    EXPECT(TypeToken::TK_RPAREN);
    return NewUnionType(vec);
  }

  ArrayType* ParseArrayType(bool* res) {
    // ArrayType := '[' ElementTypeList ']'
    // ElementTypeList := <empty> | TypeExpression | '...' TypeExpression
    //     | TypeExpression ',' ElementTypeList
    assert(token_ == TypeToken::TK_LBRACK);
    Next();
    TypeExpressions* vec = NewTypeExpressions();
    while (token_ != TypeToken::TK_RBRACK) {
      if (token_ == TypeToken::TK_REST) {
        Next();
        TypeExpression* expr = ParseTypeExpression(CHECK);
        vec->push_back(NewRest(expr));
        break;
      } else {
        TypeExpression* expr = ParseTypeExpression(CHECK);
        vec->push_back(expr);
      }
      if (token_ != TypeToken::TK_RBRACK) {
        EXPECT(TypeToken::TK_COMMA);
      }
    }
    EXPECT(TypeToken::TK_RBRACK);
    return NewArrayType(vec);
  }

  RecordType* ParseRecordType(bool* res) {
    // RecordType := '{' FieldTypeList '}'
    // FieldTypeList := FieldType | FieldType ',' FieldTypeList
    assert(token_ == TypeToken::TK_LBRACE);
    Next();
    TypeExpressions* vec = NewTypeExpressions();
    while (token_ != TypeToken::TK_RBRACE) {
      TypeExpression* field = ParseFieldType(CHECK);
      vec->push_back(field);
      if (token_ != TypeToken::TK_RBRACE) {
        EXPECT(TypeToken::TK_COMMA);
      }
    }
    return NewRecordType(vec);
  }

  TypeExpression* ParseFieldType(bool* res) {
    // FieldType := FieldName | FieldName ':' TypeExpression
    // FieldName := NameExpression | StringLiteral | NumberLiteral |
    // ReservedIdentifier
    IS(token_ == TypeToken::TK_NAME);
    NameExpression* key = ParseNameExpression(CHECK);
    if (token_ == TypeToken::TK_COLON) {
      Next();
      TypeExpression* value = ParseTypeExpression(CHECK);
      return NewFieldTypeKeyValue(key, value);
    }
    return key;
  }

  TypeName* ParseTypeName(bool* res) {
    // TypeName := NameExpression | NameExpression TypeApplication
    // TypeApplication := '.<' TypeExpressionList '>'
    // TypeExpressionList := TypeExpression // a white lie
    assert(token_ == TokenType::TK_NAME);
    NameExpression* name = ParseNameExpression(CHECK);
    if (token_ == TokenType::TK_DOT_LT) {
      Next();
      TypeExpression* application = ParseTypeExpression(CHECK);
      EXPECT(TokenType::TK_GT);
      return NewTypeNameWithApplication(name, application);
    }
    return name;
  }

  NameExpression* ParseNameExpression(bool* res) {
    NameExpression* name = NewNameExpression(lexer_.Buffer());
    lexer_.Next();
    return name;
  }

  FunctionType* ParseFunctionType(bool* res) {
    // FunctionType := 'function' FunctionSignatureType
    //
    // FunctionSignatureType :=
    //     | TypeParameters '(' ')' ResultType
    //     | TypeParameters '(' ParametersType ')' ResultType
    //     | TypeParameters '(' 'this' ':' TypeName ')' ResultType
    //     | TypeParameters '(' 'this' ':' TypeName ',' ParametersType ')' ResultType
    assert(lexer_.IsFunction());
    Next();

    // Google Closure Compiler is not implementing TypeParameters.
    // So we do not. if we don't get '(', we see it as error.
    EXPECT(TypeToken::TK_LPAREN);

    bool parameters_are_required = false;
    TypeName* this_binding = NULL;
    bool is_new = false;
    if (token_ != TypeToken::TK_RPAREN) {
      // ParametersType or 'this'
      if (token_ == TypeToken::TK_NAME &&
          (lexer_.IsThisLiteral() || lexer_.IsNewLiteral())) {
        // 'this' or 'new'
        // 'new' is Closure Compiler extension
        is_new = lexer_.IsNewLiteral();
        Next();
        EXPECT(TypeToken::TK_COLON);
        this_binding = ParseTypeName(CHECK);
        if (token_ == TypeToken::TK_COMMA) {
          Next();
          parameters_are_required = true;
        }
      } else {
        parameters_are_required = true;
      }
    }

    ParametersType* params = NULL;
    if (parameters_are_required) {
      params = ParseParametersType(CHECK);
    }
    EXPECT(TypeToken::TK_RPAREN);

    TypeExpression* result = NULL;
    if (token_ == TypeToken::TK_COLON) {
      result = ParseResultType(CHECK);
    }
    return NewFunctionType(is_new, this_binding, params, result);
  }

  ParametersType* ParseParametersType(bool* res) {
    // ParametersType := RestParameterType | NonRestParametersType
    //     | NonRestParametersType ',' RestParameterType
    //
    // RestParameterType := '...' Identifier
    //
    // NonRestParametersType := ParameterType ',' NonRestParametersType
    //     | ParameterType
    //     | OptionalParametersType
    // OptionalParametersType := OptionalParameterType
    //     | OptionalParameterType, OptionalParametersType
    // OptionalParameterType := ParameterType=
    // ParameterType := TypeExpression | Identifier ':' TypeExpression
    TypeExpressions* vec = NewTypeExpressions();
    while (token_ != TypeToken::TK_RPAREN) {
      if (token_ == TypeToken::TK_REST) {
        // RestParameterType
        Next();
        NameExpression* ident = ParseNameExpression(CHECK);
        vec->push_back(NewRestExpression(ident));
        break;
      } else {
        TypeExpression* expr = ParseTypeExpression(CHECK);
        if (token_ == TypeToken::TK_EQUAL) {
          // postfix equal '='
          Next();
          vec->push_back(NewPostfixEqualExpression(expr));
        } else {
          vec->push_back(expr);
        }
      }
      if (token_ != TypeToken::TK_RBRACK) {
        EXPECT(TypeToken::TK_COMMA);
      }
    }
    return NewParametersType(vec);
  }

  TypeExpression* ParseResultType(bool* res) {
    // ResultType := <empty> | ':' void | ':' TypeExpression
    //
    // BNF is above
    // but, we remove <empty> pattern, so token is always TypeToken::COLON
    assert(token_ == TypeToken::COLON);
    Next();
    if (token_ == TypeToken::TK_NAME && lexer_.IsVoidLiteral()) {
      return NewVoidLiteral();
    }
    return ParseTypeExpression(res);
  }

  TypeToken::Type Next() {
    return token_ = lexer_.Next();
  }

  TypeToken::Type token_;
  TypeToken::Type_ unget_;
  TypeLexer lexer_;
};

#undef CHECK

} }  // namespace az::jsdoc
#endif  // _AZ_JSDOC_TYPE_PARSER_H_
