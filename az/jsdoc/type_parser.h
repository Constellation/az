// Type Parser parse TypeExpression in JSDoc.
//
// original TypeExpression parser code is closure-compiler
//
#ifndef AZ_JSDOC_TYPE_PARSER_H_
#define AZ_JSDOC_TYPE_PARSER_H_
#include <iv/noncopyable.h>
#include <iv/ustringpiece.h>
#include <iv/character.h>
#include <iv/dtoa.h>
#include <az/factory.h>
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

class TypeParser : private iv::core::Noncopyable<TypeParser> {
 public:
  explicit TypeParser(AstFactory* factory,
                      const iv::core::UStringPiece& source)
    : factory_(factory),
      lexer_(source) {
  }

  TypeExpression* ParseParamType() {
    // top level
    bool ok = true;
    bool* res = &ok;
    Next();
    TypeExpression* expr = ParseParamType(CHECK);
    IS(TypeToken::TK_EOS);
    return expr;
  }

  TypeExpression* ParseType() {
    // top level
    bool ok = true;
    bool* res = &ok;
    Next();
    TypeExpression* expr = ParseTypeExpression(CHECK);
    IS(TypeToken::TK_EOS);
    return expr;
  }

 private:

  TypeExpression* ParseParamType(bool* res) {
    if (token_ == TypeToken::TK_REST) {
      Next();
      TypeExpression* top = ParseTopLevelTypeExpression(CHECK);
      return new (factory_) RestExpression(top);
    }
    TypeExpression* expr = ParseTopLevelTypeExpression(CHECK);
    if (token_ == TypeToken::TK_EQUAL) {
      Next();
      return new (factory_) PostfixEqualExpression(expr);
    }
    return expr;
  }

  TypeExpression* ParseTopLevelTypeExpression(bool* res) {
    // TopLevelTypeExpression := TypeExpression
    //     | TypeUnionList
    //
    // this rule is Google Closure Compiler extension
    TypeExpression* expr = ParseTypeExpression(CHECK);
    if (token_ == TypeToken::TK_PIPE) {
      // union list extension
      TypeExpressions* vec = factory_->NewVector<TypeExpression*>();
      vec->push_back(expr);
      Next();
      while (true) {
        TypeExpression* expr = ParseTypeExpression(CHECK);
        vec->push_back(expr);
        if (token_ == TypeToken::TK_PIPE) {
          Next();
        } else {
          break;
        }
      }
      return new (factory_) UnionType(vec);
    }
    return expr;
  }

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
          token_ == TypeToken::TK_PIPE ||
          token_ == TypeToken::TK_EOS) {
        // '?'
        return new (factory_) QuestionLiteral();
      }
      TypeExpression* expr = ParseBasicTypeExpression(CHECK);
      return new (factory_) PrefixQuestionExpression(expr);
    } else if (token_ == TypeToken::TK_BANG) {
      Next();
      TypeExpression* expr = ParseBasicTypeExpression(CHECK);
      return new (factory_) PrefixBangExpression(expr);
    } else {
      TypeExpression* expr = ParseBasicTypeExpression(CHECK);
      // check postfix
      if (token_ == TypeToken::TK_BANG) {
        Next();
        return new (factory_) PostfixBangExpression(expr);
      } else if (token_ == TypeToken::TK_QUESTION) {
        Next();
        return new (factory_) PostfixQuestionExpression(expr);
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
      return new (factory_) StarLiteral();
    } else if (token_ == TypeToken::TK_LPAREN) {
      return ParseUnionType(res);
    } else if (token_ == TypeToken::TK_LBRACK) {
      return ParseArrayType(res);
    } else if (token_ == TypeToken::TK_LBRACE) {
      return ParseRecordType(res);
    } else if (token_ == TypeToken::TK_NAME) {
      if (lexer_.IsNullLiteral()) {
        Next();
        return new (factory_) NullLiteral();
      } else if (lexer_.IsUndefinedLiteral()) {
        Next();
        return new (factory_) UndefinedLiteral();
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
    assert(token_ == TypeToken::TK_LPAREN);
    Next();
    TypeExpressions* vec = factory_->NewVector<TypeExpression*>();
    while (true) {
      TypeExpression* expr = ParseTypeExpression(CHECK);
      vec->push_back(expr);
      if (token_ == TypeToken::TK_PIPE) {
        Next();
      } else {
        break;
      }
    }
    EXPECT(TypeToken::TK_RPAREN);
    return new (factory_) UnionType(vec);
  }

  ArrayType* ParseArrayType(bool* res) {
    // ArrayType := '[' ElementTypeList ']'
    // ElementTypeList := <empty> | TypeExpression | '...' TypeExpression
    //     | TypeExpression ',' ElementTypeList
    assert(token_ == TypeToken::TK_LBRACK);
    Next();
    TypeExpressions* vec = factory_->NewVector<TypeExpression*>();
    while (token_ != TypeToken::TK_RBRACK) {
      if (token_ == TypeToken::TK_REST) {
        Next();
        TypeExpression* expr = ParseTypeExpression(CHECK);
        vec->push_back(new (factory_) RestExpression(expr));
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
    return new (factory_) ArrayType(vec);
  }

  RecordType* ParseRecordType(bool* res) {
    // RecordType := '{' FieldTypeList '}'
    // FieldTypeList := FieldType | FieldType ',' FieldTypeList
    assert(token_ == TypeToken::TK_LBRACE);
    Next();
    TypeExpressions* vec = factory_->NewVector<TypeExpression*>();
    while (token_ != TypeToken::TK_RBRACE) {
      TypeExpression* field = ParseFieldType(CHECK);
      vec->push_back(field);
      if (token_ != TypeToken::TK_RBRACE) {
        EXPECT(TypeToken::TK_COMMA);
      }
    }
    EXPECT(TypeToken::TK_RBRACE);
    return new (factory_) RecordType(vec);
  }

  TypeExpression* ParseFieldType(bool* res) {
    // FieldType := FieldName | FieldName ':' TypeExpression
    // FieldName := NameExpression | StringLiteral | NumberLiteral |
    // ReservedIdentifier
    NameExpression* key = ParseFieldName(CHECK);
    if (token_ == TypeToken::TK_COLON) {
      Next();
      TypeExpression* value = ParseTypeExpression(CHECK);
      return new (factory_) FieldTypeKeyValue(key, value);
    }
    return key;
  }

  NameExpression* ParseFieldName(bool* res) {
    if (token_ == TypeToken::TK_NAME || token_ == TypeToken::TK_STRING) {
      return ParseNameExpression(res);
    } else if (token_ == TypeToken::TK_NUMBER) {
      const double val = lexer_.Numeric();
      iv::core::dtoa::StringPieceDToA builder;
      builder.Build(val);
      NameString* str = factory_->NewUString(builder.buffer());
      NameExpression* name = new (factory_) NameExpression(str);
      Next();
      return name;
    } else {
      UNEXPECT(token_);
    }
  }

  TypeName* ParseTypeName(bool* res) {
    // TypeName := NameExpression | NameExpression TypeApplication
    // TypeApplication := '.<' TypeExpressionList '>'
    // TypeExpressionList := TypeExpression // a white lie
    IS(TypeToken::TK_NAME);
    NameExpression* name = ParseNameExpression(CHECK);
    if (token_ == TypeToken::TK_DOT_LT) {
      Next();
      TypeExpressions* application = ParseTypeExpressionList(CHECK);
      EXPECT(TypeToken::TK_GT);
      return new (factory_) TypeNameWithApplication(name, application);
    }
    return name;
  }

  TypeExpressions* ParseTypeExpressionList(bool* res) {
    // TypeExpressionList := TopLevelTypeExpression
    //     | TopLevelTypeExpression ',' TypeExpressionList
    TypeExpressions* vec = factory_->NewVector<TypeExpression*>();
    TypeExpression* top = ParseTopLevelTypeExpression(CHECK);
    vec->push_back(top);
    while (token_ == TypeToken::TK_COMMA) {
      Next();
      top = ParseTopLevelTypeExpression(CHECK);
      vec->push_back(top);
    }
    return vec;
  }

  NameExpression* ParseNameExpression(bool* res) {
    assert(token_ == TypeToken::TK_NAME || token_ == TypeToken::TK_STRING);
    NameString* str = factory_->NewUString(lexer_.Buffer());
    NameExpression* name = new (factory_) NameExpression(str);
    Next();
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
    return new (factory_) FunctionType(is_new, this_binding, params, result);
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
    TypeExpressions* vec = factory_->NewVector<TypeExpression*>();
    while (token_ != TypeToken::TK_RPAREN) {
      if (token_ == TypeToken::TK_REST) {
        // RestParameterType
        Next();
        if (token_ == TypeToken::TK_RPAREN) {
          vec->push_back(new (factory_) RestExpression(NULL));
        } else {
          EXPECT(TypeToken::TK_LBRACK);
          IS(TypeToken::TK_NAME);
          NameExpression* ident = ParseNameExpression(CHECK);
          vec->push_back(new (factory_) RestExpression(ident));
          EXPECT(TypeToken::TK_RBRACK);
        }
        break;
      } else {
        TypeExpression* expr = ParseTypeExpression(CHECK);
        if (token_ == TypeToken::TK_EQUAL) {
          // postfix equal '='
          Next();
          vec->push_back(new (factory_) PostfixEqualExpression(expr));
        } else {
          vec->push_back(expr);
        }
      }
      if (token_ != TypeToken::TK_RPAREN) {
        EXPECT(TypeToken::TK_COMMA);
      }
    }
    return new (factory_) ParametersType(vec);
  }

  TypeExpression* ParseResultType(bool* res) {
    // ResultType := <empty> | ':' void | ':' TypeExpression
    //
    // BNF is above
    // but, we remove <empty> pattern, so token is always TypeToken::COLON
    assert(token_ == TypeToken::TK_COLON);
    Next();
    if (token_ == TypeToken::TK_NAME && lexer_.IsVoidLiteral()) {
      return new (factory_) VoidLiteral();
    }
    return ParseTypeExpression(res);
  }

  TypeToken::Type Next() {
    return token_ = lexer_.Next();
  }

  AstFactory* factory_;
  TypeLexer lexer_;
  TypeToken::Type token_;
};

#undef IS
#undef EXPECT
#undef UNEXPECT
#undef CHECK

} }  // namespace az::jsdoc
#endif  // AZ_JSDOC_TYPE_PARSER_H_
