// Az parser
// this is incremental parser for Az project only
// original source code is iv::core::Parser
#ifndef AZ_PARSER_H_
#define AZ_PARSER_H_
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <iv/detail/unordered_map.h>
#include <iv/detail/unordered_set.h>
#include <iv/detail/type_traits.h>
#include <iv/detail/array.h>
#include <iv/static_assert.h>
#include <iv/maybe.h>
#include <iv/ast.h>
#include <iv/ast_factory.h>
#include <iv/lexer.h>
#include <iv/dtoa.h>
#include <iv/noncopyable.h>
#include <iv/utils.h>
#include <iv/ustring.h>
#include <iv/enable_if.h>
#include <iv/none.h>
#include <az/ast_fwd.h>
#include <az/token.h>
#include <az/skip.h>
#include <az/context.h>
#include <az/debug_log.h>
#include <az/complete_lexer.h>
#include <az/jsdoc/provider.h>
namespace az {

using iv::core::Token;

#define IS(token)\
  do {\
    if (token_ != token) {\
      *res = false;\
      ReportUnexpectedToken(token);\
      errors_.push_back(error_);\
      error_.clear();\
      return NULL;\
    }\
  } while (0)

#define IS_NORETURN(token)\
  do {\
    if (token_ != token) {\
      *res = false;\
      ReportUnexpectedToken(token);\
      errors_.push_back(error_);\
      error_.clear();\
    }\
  } while (0);\
  if (!*res)

#define EXPECT(token)\
  do {\
    if (token_ != token) {\
      *res = false;\
      ReportUnexpectedToken(token);\
      errors_.push_back(error_);\
      error_.clear();\
      return NULL;\
    }\
    Next();\
  } while (0)

#define UNEXPECT(token)\
  do {\
    *res = false;\
    ReportUnexpectedToken(token);\
    errors_.push_back(error_);\
    error_.clear();\
    return NULL;\
  } while (0)

#define RAISE_WITH(str, val)\
  do {\
    *res = false;\
    error_state_ |= kNotRecoverable;\
    error_.append(str);\
    errors_.push_back(error_);\
    error_.clear();\
    return val;\
  } while (0)

#define RAISE(str)\
  do {\
    *res = false;\
    error_state_ |= kNotRecoverable;\
    error_.append(str);\
    errors_.push_back(error_);\
    error_.clear();\
    return NULL;\
  } while (0)

#define RAISE_RECOVERVABLE(str)\
  do {\
    *res = false;\
    error_.append(str);\
    errors_.push_back(error_);\
    error_.clear();\
    return NULL;\
  } while (0)

#define RAISE_WITH_NUMBER(str, line)\
  do {\
    *res = false;\
    error_state_ |= kNotRecoverable;\
    error_.append(str);\
    errors_.push_back(error_);\
    error_.clear();\
    return NULL;\
  } while (0)

#define CHECK  res);\
  if (!*res) {\
    return NULL;\
  }\
  ((void)0
#define DUMMY )  // to make indentation work
#undef DUMMY

namespace detail {

static const iv::core::UString kUseStrict = iv::core::ToUString("use strict");

}  // namespace detail

template<typename Source, typename Lexer, typename Reporter, typename Completer>
class Parser : private iv::core::Noncopyable<> {
 public:
  typedef Parser<Source, Lexer, Reporter, Completer> this_type;
  typedef this_type parser_type;
  typedef Lexer lexer_type;
  typedef BasicSkip<lexer_type> Skip;

  enum ErrorState {
    kNotRecoverable = 1
  };

  class Target : private iv::core::Noncopyable<> {
   public:
    enum Type {
      kNamedOnlyStatement = 0,  // (00)2
      kIterationStatement = 2,  // (10)2
      kSwitchStatement = 3      // (11)2
    };
    Target(parser_type* parser, Type type)
      : parser_(parser),
        prev_(parser->target()),
        labels_(parser->labels()),
        node_(NULL),
        type_(type) {
      parser_->set_target(this);
      parser_->set_labels(NULL);
    }
    ~Target() {
      parser_->set_target(prev_);
    }
    inline Target* previous() const {
      return prev_;
    }
    inline bool IsAnonymous() const {
      return type_ & 2;
    }
    inline bool IsIteration() const {
      return type_ == kIterationStatement;
    }
    inline bool IsSwitch() const {
      return type_ == kSwitchStatement;
    }
    inline bool IsNamedOnly() const {
      return !IsAnonymous();
    }
    inline BreakableStatement** node() {
      if (!node_) {
        node_ = parser_->factory()->template NewPtr<BreakableStatement>();
      }
      return node_;
    }
    inline Symbols* labels() const {
      return labels_;
    }
    inline void set_node(BreakableStatement* node) {
      if (node_) {
        *node_ = node;
      }
    }
   private:
    parser_type* parser_;
    Target* prev_;
    Symbols* labels_;
    BreakableStatement** node_;
    int type_;
  };

  class TargetSwitcher : private iv::core::Noncopyable<> {
   public:
    explicit TargetSwitcher(parser_type* parser)
      : parser_(parser),
        target_(parser->target()),
        labels_(parser->labels()) {
      parser_->set_target(NULL);
      parser_->set_labels(NULL);
    }
    ~TargetSwitcher() {
      parser_->set_target(target_);
      parser_->set_labels(labels_);
    }
   private:
    parser_type* parser_;
    Target* target_;
    Symbols* labels_;
  };

  Parser(Context* ctx,
         AstFactory* factory,
         const Source& source,
         lexer_type* lexer,
         Reporter* reporter,
         Completer* completer,
         const StructuredSource& structured)
    : ctx_(ctx),
      lexer_(lexer),
      completion_point_(false),
      error_(),
      strict_(false),
      error_state_(0),
      factory_(factory),
      reporter_(reporter),
      completer_(completer),
      structured_(structured),
      scope_(NULL),
      target_(NULL),
      labels_(NULL) {
  }

// Program
//   : SourceElements
  FunctionLiteral* ParseProgram() {
    Assigneds* params = factory_->template NewVector<Assigned*>();
    Statements* body = factory_->template NewVector<Statement*>();
    Scope* const scope = factory_->NewScope(FunctionLiteral::GLOBAL);
    assert(target_ == NULL);
    bool error_flag = true;
    bool *res = &error_flag;
    const ScopeSwitcher scope_switcher(this, scope);
    Next();
    const bool strict = ParseSourceElements(Token::TK_EOS, body, res);
    assert(error_flag);  // always true. because, end token is Token::TK_EOS
    const std::size_t end_position = lexer_->end_position();
    FunctionLiteral* global =
        factory_->NewFunctionLiteral(FunctionLiteral::GLOBAL,
                                     NULL,
                                     params,
                                     body,
                                     scope,
                                     strict,
                                     0,
                                     end_position,
                                     0,
                                     end_position);
    if (completer_ &&
        completer_->HasCompletionPoint() &&
        !completer_->HasTargetFunction()) {
      completer_->RegisterTargetFunction(global);
    }
    return global;
  }

// SourceElements
//   : SourceElement
//   | SourceElement SourceElements
//
// SourceElement
//   : Statements
//   | FunctionDeclaration
  bool ParseSourceElements(Token::Type end, Statements* body, bool *res) {
    Statement* stmt;
    const StrictSwitcher strict_switcher(this);

    // directive prologue
    {
      bool octal_escaped_directive_found = false;
      std::size_t line = 0;
      while (token_ != end && token_ != Token::TK_EOS) {
        if (token_ != Token::TK_STRING) {
          // this is not directive
          break;
        }
        const typename lexer_type::State state = lexer_->StringEscapeType();
        if (!octal_escaped_directive_found && state == lexer_type::OCTAL) {
            // octal escaped string literal
            octal_escaped_directive_found = true;
            line = lexer_->line_number();
        }
        stmt = ParseStatement(res);
        body->push_back(stmt);
        if (stmt->AsExpressionStatement() &&
            stmt->AsExpressionStatement()->expr()->AsStringLiteral()) {
          Expression* const expr = stmt->AsExpressionStatement()->expr();
          // expression is directive
          if (!strict_switcher.IsStrict() &&
              state == lexer_type::NONE &&
              expr->AsStringLiteral()->value().compare(detail::kUseStrict.data()) == 0) {
            strict_switcher.SwitchStrictMode();
            if (octal_escaped_directive_found) {
              ReportAndRecovery(
                  "octal escape sequence not allowed in strict code",
                  lexer_->previous_end_position(),
                  line);
            }
            // and one token lexed is not in strict
            // so rescan
            if (token_ == Token::TK_IDENTIFIER) {
              typedef iv::core::Keyword<iv::core::IdentifyReservedWords> KeywordChecker;
              token_ = KeywordChecker::Detect(lexer_->Buffer(), true);
              break;
            }
          } else {
            // other directive
          }
        } else {
          // not directive, like
          // "String", "Comma"
          break;
        }
      }
    }

    // statements
    while (token_ != end && token_ != Token::TK_EOS) {
      if (token_ == Token::TK_FUNCTION) {
        // FunctionDeclaration
        stmt = ParseFunctionDeclaration(res);
        body->push_back(stmt);
      } else {
        // heuristic end of Statement
        stmt = ParseStatement(res);
        body->push_back(stmt);
      }
    }
    if (token_ != end) {
      ReportAndRecoveryUnexpectedToken(token_, lexer_->previous_end_position());
    }
    return strict_switcher.IsStrict();
  }

//  Statement
//    : Block
//    | FunctionStatement    // This is not standard.
//    | VariableStatement
//    | EmptyStatement
//    | ExpressionStatement
//    | IfStatement
//    | IterationStatement
//    | ContinueStatement
//    | BreakStatement
//    | ReturnStatement
//    | WithStatement
//    | LabelledStatement
//    | SwitchStatement
//    | ThrowStatement
//    | TryStatement
//    | DebuggerStatement
  Statement* ParseStatement(bool *res) {
    Statement *result = NULL;
    switch (token_) {
      case Token::TK_LBRACE:
        // Block
        result = ParseBlock(res);
        break;

      case Token::TK_CONST:
        if (strict_) {
          ReportAndRecovery(
              "\"const\" not allowed in strict code",
              lexer_->begin_position());
        }
      case Token::TK_VAR:
        // VariableStatement
        result = ParseVariableStatement(res);
        break;

      case Token::TK_SEMICOLON:
        // EmptyStatement
        result = ParseEmptyStatement();
        break;

      case Token::TK_IF:
        // IfStatement
        result = ParseIfStatement(res);
        break;

      case Token::TK_DO:
        // IterationStatement
        // do while
        result = ParseDoWhileStatement(res);
        break;

      case Token::TK_WHILE:
        // IterationStatement
        // while
        result = ParseWhileStatement(res);
        break;

      case Token::TK_FOR:
        // IterationStatement
        // for
        result = ParseForStatement(res);
        break;

      case Token::TK_CONTINUE:
        // ContinueStatement
        result = ParseContinueStatement(res);
        break;

      case Token::TK_BREAK:
        // BreakStatement
        result = ParseBreakStatement(res);
        break;

      case Token::TK_RETURN:
        // ReturnStatement
        result = ParseReturnStatement(res);
        break;

      case Token::TK_WITH:
        // WithStatement
        result = ParseWithStatement(res);
        break;

      case Token::TK_SWITCH:
        // SwitchStatement
        result = ParseSwitchStatement(res);
        break;

      case Token::TK_THROW:
        // ThrowStatement
        result = ParseThrowStatement(res);
        break;

      case Token::TK_TRY:
        // TryStatement
        result = ParseTryStatement(res);
        break;

      case Token::TK_DEBUGGER:
        // DebuggerStatement
        result = ParseDebuggerStatement(res);
        break;

      case Token::TK_FUNCTION:
        // FunctionStatement (not in ECMA-262 5th)
        // FunctionExpression
        result = ParseFunctionStatement(res);
        break;

      case Token::TK_IDENTIFIER:
        // LabelledStatement or ExpressionStatement
        result = ParseExpressionOrLabelledStatement(res);
        break;

      default:
        // ExpressionStatement or ILLEGAL
        if (IsExpressionStartToken(token_)) {
          result = ParseExpressionStatement(res);
        } else if (token_ == Token::TK_ILLEGAL) {
          // invalid token...
          ReportAndRecoveryUnexpectedToken(token_, lexer_->begin_position());
          SkipUntilSemicolonOrLineTerminator();
          Statement* stmt = factory_->NewEmptyStatement(lexer_->begin_position(), lexer_->previous_end_position());
          stmt->set_is_failed_node(true);
          result = stmt;
        } else {
          // not statement start token
          ReportAndRecoveryUnexpectedToken(token_, lexer_->begin_position());
          Statement* stmt = factory_->NewEmptyStatement(lexer_->begin_position(),
                                                        lexer_->end_position());
          stmt->set_is_failed_node(true);
          Next();
          result = stmt;
        }
        break;
    }
    return result;
  }

//  FunctionDeclaration
//    : FUNCTION IDENTIFIER '(' FormalParameterList_opt ')' '{' FunctionBody '}'
//
//  FunctionStatement
//    : FUNCTION IDENTIFIER '(' FormalParameterList_opt ')' '{' FunctionBody '}'
//
//  FunctionExpression
//    : FUNCTION
//      IDENTIFIER_opt '(' FormalParameterList_opt ')' '{' FunctionBody '}'
//
//  FunctionStatement is not standard, but implemented in SpiderMonkey
//  and this statement is very useful for not breaking FunctionDeclaration.
  Statement* ParseFunctionDeclaration(bool *res) {
    assert(token_ == Token::TK_FUNCTION);
    const std::size_t begin = lexer_->begin_position();
    FunctionLiteral* const expr = ParseFunctionLiteral(
        FunctionLiteral::DECLARATION,
        FunctionLiteral::GENERAL, res);
    if (!*res) {
      // TODO(Constellation) searching } is better ?
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    // define named function as FunctionDeclaration
    scope_->AddFunctionDeclaration(expr);
    assert(expr);
    return factory_->NewFunctionDeclaration(expr);
  }

//  Block
//    : '{' '}'
//    | '{' StatementList '}'
//
//  StatementList
//    : Statement
//    | StatementList Statement
  Block* ParseBlock(bool *res) {
    assert(token_ == Token::TK_LBRACE);
    const std::size_t begin = lexer_->begin_position();
    Statements* const body = factory_->template NewVector<Statement*>();
    Target target(this, Target::kNamedOnlyStatement);

    Next();
    while (token_ != Token::TK_RBRACE && token_ != Token::TK_EOS) {
      Statement* const stmt = ParseStatement(res);
      body->push_back(stmt);
    }
    const bool failed = (token_ == Token::TK_EOS);
    if (failed) {
      ReportAndRecoveryUnexpectedToken(token_, begin);
    }
    Next();
    assert(body);
    Block* const block = factory_->NewBlock(body,
                                            begin,
                                            lexer_->previous_end_position());
    target.set_node(block);
    block->set_is_failed_node(failed);
    return block;
  }

//  VariableStatement
//    : VAR VariableDeclarationList ';'
//    : CONST VariableDeclarationList ';'
  Statement* ParseVariableStatement(bool *res) {
    assert(token_ == Token::TK_VAR || token_ == Token::TK_CONST);
    const Token::Type op = token_;
    const std::size_t begin = lexer_->begin_position();
    bool failed = true;
    Declarations* const decls = factory_->template NewVector<Declaration*>();
    ParseVariableDeclarations(decls, token_ == Token::TK_CONST, true, res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
    }
    if (decls->empty()) {
      return ReturnFailedStatement(begin);
    }
    ExpectSemicolon(res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      failed = true;
    }
    assert(decls);
    Statement* stmt = factory_->NewVariableStatement(op,
                                                     decls,
                                                     begin,
                                                     lexer_->previous_end_position());
    stmt->set_is_failed_node(failed);
    return stmt;
  }

//  VariableDeclarationList
//    : VariableDeclaration
//    | VariableDeclarationList ',' VariableDeclaration
//
//  VariableDeclaration
//    : IDENTIFIER Initialiser_opt
//
//  Initialiser_opt
//    :
//    | Initialiser
//
//  Initialiser
//    : '=' AssignmentExpression
  Declarations* ParseVariableDeclarations(Declarations* decls,
                                          bool is_const,
                                          bool contains_in,
                                          bool *res) {
    Declaration* decl;
    do {
      Next();
      std::shared_ptr<jsdoc::Info> info = GetAndResetJSDocInfo();
      IS(Token::TK_IDENTIFIER);
      const iv::core::ast::SymbolHolder name = ParseSymbol();
      Assigned* assigned = factory_->NewAssigned(name);
      if (info) {
        ctx_->Tag(assigned, info);
      }
      // section 12.2.1
      // within the strict code, Identifier must not be "eval" or "arguments"
      if (strict_) {
        const EvalOrArguments val = IsEvalOrArguments(name);
        if (val) {
          if (val == kEval) {
            RAISE("assignment to \"eval\" not allowed in strict code");
          } else {
            assert(val == kArguments);
            RAISE("assignment to \"arguments\" not allowed in strict code");
          }
        }
      }

      if (token_ == Token::TK_ASSIGN) {
        Next();
        // AssignmentExpression
        Expression* expr = ParseAssignmentExpression(contains_in, CHECK);
        decl = factory_->NewDeclaration(assigned, expr);
      } else {
        // Undefined Expression
        decl = factory_->NewDeclaration(assigned, NULL);
      }
      decls->push_back(decl);
      scope_->AddUnresolved(assigned, is_const);
    } while (token_ == Token::TK_COMMA);
    return decls;
  }

//  EmptyStatement
//    : ';'
  Statement* ParseEmptyStatement() {
    assert(token_ == Token::TK_SEMICOLON);
    Next();
    return factory_->NewEmptyStatement(lexer_->previous_begin_position(),
                                       lexer_->previous_end_position());
  }

//  IfStatement
//    : IF '(' Expression ')' Statement ELSE Statement
//    | IF '(' Expression ')' Statement
  Statement* ParseIfStatement(bool *res) {
    assert(token_ == Token::TK_IF);
    const std::size_t begin = lexer_->begin_position();
    Statement* else_statement = NULL;
    Next();

    if (!ConsumeOrRecovery<Token::TK_LPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    bool failed = false;
    Expression* expr = ParseExpression(true, res);
    if (!*res) {
      // if invalid WithStatement is like,
      // with () {  <= error occurred, but token RPAREN is found.
      // }
      // through this error and parse WithStatement body
      reporter_->ReportSyntaxError(errors_.back(), begin);
      if (token_ != Token::TK_RPAREN) {
        SkipUntilSemicolonOrLineTerminator();
      }
      *res = true;  // recovery
      failed = true;
      if (!expr) {
        expr = MakeFailedExpression();
      }
    }

    if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    Statement* const then_statement = ParseStatement(res);
    if (token_ == Token::TK_ELSE) {
      Next();
      else_statement = ParseStatement(res);
    }
    assert(expr && then_statement);
    Statement* stmt = factory_->NewIfStatement(expr,
                                               then_statement,
                                               else_statement,
                                               begin);
    stmt->set_is_failed_node(failed);
    return stmt;
  }

//  IterationStatement
//    : DO Statement WHILE '(' Expression ')' ';'
//    | WHILE '(' Expression ')' Statement
//    | FOR '(' ExpressionNoIn_opt ';' Expression_opt ';' Expression_opt ')'
//      Statement
//    | FOR '(' VAR VariableDeclarationListNoIn ';'
//              Expression_opt ';'
//              Expression_opt ')'
//              Statement
//    | FOR '(' LeftHandSideExpression IN Expression ')' Statement
//    | FOR '(' VAR VariableDeclarationNoIn IN Expression ')' Statement
  Statement* ParseDoWhileStatement(bool *res) {
    //  DO Statement WHILE '(' Expression ')' ';'
    assert(token_ == Token::TK_DO);
    const std::size_t begin = lexer_->begin_position();
    Target target(this, Target::kIterationStatement);
    Next();

    Statement* const body = ParseStatement(res);

    IS_NORETURN(Token::TK_WHILE) {
      // while is not found
      // if Statement body is valid, skip
      reporter_->ReportSyntaxError(errors_.back(), begin);
      *res = true;  // recovery
      if (body->IsFailed()) {
        SkipUntilSemicolonOrLineTerminator();
      }
      return ReturnFailedStatement(begin);
    }

    Next();

    if (!ConsumeOrRecovery<Token::TK_LPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    bool failed = false;
    Expression* expr = ParseExpression(true, res);
    if (!*res) {
      // if invalid DoWhileStatement is like,
      // do {
      // } while (....) <= error occurred, but token RPAREN is found.
      // through this error and parse WithStatement body
      reporter_->ReportSyntaxError(errors_.back(), begin);
      if (token_ != Token::TK_RPAREN) {
        SkipUntilSemicolonOrLineTerminator();
      }
      *res = true;  // recovery
      failed = true;
      if (!expr) {
        expr = MakeFailedExpression();
      }
    }

    if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    // ex:
    //   do {
    //     print("valid syntax");
    //   } while (0) return true;
    // is syntax valid
    if (token_ == Token::TK_SEMICOLON) {
      Next();
    }

    assert(body && expr);
    DoWhileStatement* const stmt = factory_->NewDoWhileStatement(
        body, expr, begin, lexer_->previous_end_position());
    target.set_node(stmt);
    stmt->set_is_failed_node(failed);
    return stmt;
  }

//  WHILE '(' Expression ')' Statement
  Statement* ParseWhileStatement(bool *res) {
    assert(token_ == Token::TK_WHILE);
    const std::size_t begin = lexer_->begin_position();
    bool failed = false;
    Next();

    if (!ConsumeOrRecovery<Token::TK_LPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    Expression* expr = ParseExpression(true, res);
    if (!*res) {
      // if invalid WithStatement is like,
      // with () {  <= error occurred, but token RPAREN is found.
      // }
      // through this error and parse WithStatement body
      reporter_->ReportSyntaxError(errors_.back(), begin);
      if (token_ != Token::TK_RPAREN) {
        SkipUntilSemicolonOrLineTerminator();
      }
      *res = true;  // recovery
      failed = true;
    }
    Target target(this, Target::kIterationStatement);

    if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    Statement* const body = ParseStatement(res);
    if (!expr) {
      expr = MakeFailedExpression();
    }
    assert(expr && body);
    WhileStatement* const stmt = factory_->NewWhileStatement(body,
                                                             expr, begin);
    stmt->set_is_failed_node(failed);
    target.set_node(stmt);
    return stmt;
  }

//  FOR '(' ExpressionNoIn_opt ';' Expression_opt ';' Expression_opt ')'
//  Statement
//  FOR '(' VAR VariableDeclarationListNoIn ';'
//          Expression_opt ';'
//          Expression_opt ')'
//          Statement
//  FOR '(' LeftHandSideExpression IN Expression ')' Statement
//  FOR '(' VAR VariableDeclarationNoIn IN Expression ')' Statement
  Statement* ParseForStatement(bool *res) {
    assert(token_ == Token::TK_FOR);
    const std::size_t for_stmt_begin = lexer_->begin_position();
    bool failed = false;
    Next();

    if (!ConsumeOrRecovery<Token::TK_LPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
      return ReturnFailedStatement(for_stmt_begin);
    }

    Statement *init = NULL;

    if (token_ != Token::TK_SEMICOLON) {
      if (token_ == Token::TK_VAR || token_ == Token::TK_CONST) {
        const std::size_t begin = lexer_->begin_position();
        const Token::Type op = token_;
        Declarations* const decls =
            factory_->template NewVector<Declaration*>();
        ParseVariableDeclarations(decls, token_ == Token::TK_CONST, false, res);
        if (!*res) {
          reporter_->ReportSyntaxError(errors_.back(), begin);
          *res = true;  // recovery
          failed = true;
        }
        if (decls->empty()) {
          return ReturnFailedStatement(begin);
        }
        if (token_ == Token::TK_IN) {
          assert(decls);
          if (decls->size() != 1) {
            // ForInStatement requests VaraibleDeclarationNoIn (not List),
            // so check declarations' size is 1.
            ReportAndRecovery("invalid for-in left-hand-side", begin);
            failed = true;
          }
          VariableStatement* const var =
              factory_->NewVariableStatement(op,
                                             decls,
                                             begin,
                                             lexer_->previous_end_position());
          var->set_is_failed_node(failed);
          init = var;
          // for in loop
          Next();

          Expression* enumerable = ParseExpression(true, res);
          if (!*res) {
            reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
            if (token_ != Token::TK_RPAREN) {
              SkipUntilSemicolonOrLineTerminator();
            }
            *res = true;  // recovery
            failed = true;
            if (!enumerable) {
              enumerable = MakeFailedExpression();
            }
          }

          if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
            reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
            return ReturnFailedStatement(for_stmt_begin);
          }

          Target target(this, Target::kIterationStatement);
          // ParseStatement never fail
          Statement* const body = ParseStatement(res);
          assert(body && init && enumerable);
          ForInStatement* const forstmt =
              factory_->NewForInStatement(body, init, enumerable,
                                          for_stmt_begin);
          forstmt->set_is_failed_node(failed);
          target.set_node(forstmt);
          return forstmt;
        } else {
          assert(decls);
          init = factory_->NewVariableStatement(op, decls,
                                                begin,
                                                lexer_->end_position());
        }
      } else {
        Expression* init_expr = ParseExpression(false, res);
        if (!*res) {
          reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
          // TODO(Constellation) insert effective skip to this line
          *res = true;  // recovery
          failed = true;
          if (!init_expr) {
            init_expr = MakeFailedExpression();
          }
        }

        if (token_ == Token::TK_IN) {
          // for in loop
          assert(init_expr);
          init = factory_->NewExpressionStatement(init_expr,
                                                  lexer_->previous_end_position());
          if (!init_expr->IsValidLeftHandSide() && !failed) {
            ReportAndRecovery("invalid for-in left-hand-side", for_stmt_begin);
            failed = true;
          }
          Next();

          Expression* enumerable = ParseExpression(true, res);
          if (!*res) {
            reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
            if (token_ != Token::TK_RPAREN) {
              SkipUntilSemicolonOrLineTerminator();
            }
            *res = true;  // recovery
            failed = true;
            if (!enumerable) {
              enumerable = MakeFailedExpression();
            }
          }

          if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
            reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
            return ReturnFailedStatement(for_stmt_begin);
          }

          Target target(this, Target::kIterationStatement);
          // ParseStatement never fail
          Statement* const body = ParseStatement(res);
          assert(body && init && enumerable);
          ForInStatement* const forstmt =
              factory_->NewForInStatement(body, init, enumerable,
                                          for_stmt_begin);
          forstmt->set_is_failed_node(failed);
          target.set_node(forstmt);
          return forstmt;
        } else {
          assert(init_expr);
          init = factory_->NewExpressionStatement(init_expr,
                                                  lexer_->end_position());
        }
      }
    }

    // not for-in statement
    // ordinary for loop
    if (!ConsumeOrRecovery<Token::TK_SEMICOLON>()) {
      reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
      if (token_ != Token::TK_RPAREN) {
        SkipUntilSemicolonOrLineTerminator();
      }
      *res = true;  // recovery
      failed = true;
    }

    Expression* cond = NULL;
    if (token_ == Token::TK_SEMICOLON) {
      // no cond expr => for (...;;
      Next();
    } else {
      cond = ParseExpression(true, res);
      if (!*res) {
        reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
        if (token_ != Token::TK_RPAREN) {
          SkipUntil(Token::TK_RPAREN);
        }
        *res = true;  // recovery
        failed = true;
        if (!cond) {
          cond = MakeFailedExpression();
        }
      }
      if (token_ == Token::TK_SEMICOLON) {
        Next();
      }
    }

    Expression* next = NULL;
    if (token_ == Token::TK_RPAREN) {
      Next();
    } else {
      next = ParseExpression(true, res);
      if (!*res) {
        reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
        if (token_ != Token::TK_RPAREN) {
          SkipUntil(Token::TK_RPAREN);
        }
        *res = true;  // recovery
        failed = true;
        if (!next) {
          next = MakeFailedExpression();
        }
      }
      assert(next);
      if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
        reporter_->ReportSyntaxError(errors_.back(), for_stmt_begin);
        return ReturnFailedStatement(for_stmt_begin);
      }
    }

    Target target(this, Target::kIterationStatement);
    Statement* const body = ParseStatement(res);
    assert(body);
    ForStatement* const forstmt =
        factory_->NewForStatement(body, init, cond, next, for_stmt_begin);
    target.set_node(forstmt);
    forstmt->set_is_failed_node(failed);
    return forstmt;
  }

//  ContinueStatement
//    : CONTINUE Identifier_opt ';'
  Statement* ParseContinueStatement(bool *res) {
    // TODO(Constellation) refactoring duplicate procedure
    assert(token_ == Token::TK_CONTINUE);
    const std::size_t begin = lexer_->begin_position();
    iv::core::ast::SymbolHolder label;
    IterationStatement** target;
    Next();
    if (!lexer_->has_line_terminator_before_next() &&
        token_ != Token::TK_SEMICOLON &&
        token_ != Token::TK_RBRACE &&
        token_ != Token::TK_EOS) {
      if (!CheckOrRecovery<Token::TK_IDENTIFIER>()) {
        reporter_->ReportSyntaxError(errors_.back(), begin);
        return ReturnFailedStatement(begin);
      }

      label = ParseSymbol();
      target = LookupContinuableTarget(label);
      if (!target) {
        ReportAndRecovery("label not found", begin);
        ExpectSemicolon(res);
        if (!*res) {
          reporter_->ReportSyntaxError(errors_.back(), begin);
          SkipUntilSemicolonOrLineTerminator();
          *res = true;  // recovery
          return ReturnFailedStatement(begin);
        } else {
          // ExpectSemicolon not failed
          return ReturnFailedStatement(begin);
        }
      }
    } else {
      target = LookupContinuableTarget();
      if (!target) {
        ReportAndRecovery("label not found", begin);
        ExpectSemicolon(res);
        if (!*res) {
          reporter_->ReportSyntaxError(errors_.back(), begin);
          SkipUntilSemicolonOrLineTerminator();
          *res = true;  // recovery
        }
        return ReturnFailedStatement(begin);
      }
    }
    ExpectSemicolon(res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      Statement* stmt = factory_->NewContinueStatement(label, target, begin, lexer_->previous_end_position());
      stmt->set_is_failed_node(true);
      return stmt;
    }
    // ExpectSemicolon not failed
    Statement* stmt = factory_->NewContinueStatement(label, target, begin,
                                                     lexer_->previous_end_position());
    return stmt;
  }

//  BreakStatement
//    : BREAK Identifier_opt ';'
  Statement* ParseBreakStatement(bool *res) {
    assert(token_ == Token::TK_BREAK);
    const std::size_t begin = lexer_->begin_position();
    iv::core::ast::SymbolHolder label;
    BreakableStatement** target = NULL;
    Next();
    if (!lexer_->has_line_terminator_before_next() &&
        token_ != Token::TK_SEMICOLON &&
        token_ != Token::TK_RBRACE &&
        token_ != Token::TK_EOS) {
      // label
      if (!CheckOrRecovery<Token::TK_IDENTIFIER>()) {
        reporter_->ReportSyntaxError(errors_.back(), begin);
        return ReturnFailedStatement(begin);
      }

      label = ParseSymbol();
      if (ContainsLabel(labels_, label)) {
        // example
        //
        //   do {
        //     test: break test;
        //   } while (0);
        //
        // This BreakStatement is interpreted as EmptyStatement
        // In iv, BreakStatement with label, but without target is
        // interpreted as EmptyStatement
      } else {
        target = LookupBreakableTarget(label);
        if (!target) {
          ReportAndRecovery("label not found", begin);
          ExpectSemicolon(res);
          if (!*res) {
            reporter_->ReportSyntaxError(errors_.back(), begin);
            SkipUntilSemicolonOrLineTerminator();
            *res = true;  // recovery
            return ReturnFailedStatement(begin);
          } else {
            // ExpectSemicolon not failed
            return ReturnFailedStatement(begin);
          }
        }
      }
    } else {
      target = LookupBreakableTarget();
      if (!target) {
        ReportAndRecovery("label not found", begin);
        ExpectSemicolon(res);
        if (!*res) {
          reporter_->ReportSyntaxError(errors_.back(), begin);
          SkipUntilSemicolonOrLineTerminator();
          *res = true;  // recovery
        }
        return ReturnFailedStatement(begin);
      }
    }
    ExpectSemicolon(res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      Statement* stmt = factory_->NewBreakStatement(label, target, begin, lexer_->previous_end_position());
      stmt->set_is_failed_node(true);
      return stmt;
    }
    Statement* stmt = factory_->NewBreakStatement(label, target, begin,
                                                  lexer_->previous_end_position());
    return stmt;
  }

//  ReturnStatement
//    : RETURN Expression_opt ';'
  Statement* ParseReturnStatement(bool *res) {
    assert(token_ == Token::TK_RETURN);
    const std::size_t begin = lexer_->begin_position();
    Next();

    bool failed = false;

    if (scope_->IsGlobal()) {
      // return statement found in global
      // SyntaxError
      ReportAndRecovery("\"return\" not in function", begin);
      failed = true;
    }

    if (lexer_->has_line_terminator_before_next() ||
        token_ == Token::TK_SEMICOLON ||
        token_ == Token::TK_RBRACE ||
        token_ == Token::TK_EOS) {
      // always pass
      ExpectSemicolon(res);
      assert(*res);
      Statement* stmt = factory_->NewReturnStatement(NULL,
                                                     begin,
                                                     lexer_->previous_end_position());
      stmt->set_is_failed_node(failed);
      return stmt;
    }
    Expression* const expr = ParseExpression(true, res);
    if (!*res) {
      // expression error occurred
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    ExpectSemicolon(res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    return factory_->NewReturnStatement(expr, begin,
                                        lexer_->previous_end_position());
  }

//  WithStatement
//    : WITH '(' Expression ')' Statement
  Statement* ParseWithStatement(bool *res) {
    assert(token_ == Token::TK_WITH);
    const std::size_t begin = lexer_->begin_position();
    bool failed = false;
    Next();

    // section 12.10.1
    // when in strict mode code, WithStatement is not allowed.
    if (strict_) {
      ReportAndRecovery("with statement not allowed in strict code", begin);
      failed = true;
    }

    if (!ConsumeOrRecovery<Token::TK_LPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    Expression* expr = ParseExpression(true, res);
    if (!*res) {
      // if invalid WithStatement is like,
      // with () {  <= error occurred, but token RPAREN is found.
      // }
      // through this error and parse WithStatement body
      reporter_->ReportSyntaxError(errors_.back(), begin);
      if (token_ != Token::TK_RPAREN) {
        SkipUntilSemicolonOrLineTerminator();
      }
      *res = true;  // recovery
      failed = true;
    }

    if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    Statement* const body = ParseStatement(res);
    if (!expr) {
      expr = MakeFailedExpression();
    }
    assert(expr && body);
    Statement* const stmt = factory_->NewWithStatement(expr, body, begin);
    stmt->set_is_failed_node(failed);
    return stmt;
  }

//  SwitchStatement
//    : SWITCH '(' Expression ')' CaseBlock
//
//  CaseBlock
//    : '{' CaseClauses_opt '}'
//    | '{' CaseClauses_opt DefaultClause CaseClauses_opt '}'
  Statement* ParseSwitchStatement(bool *res) {
    assert(token_ == Token::TK_SWITCH);
    const std::size_t begin = lexer_->begin_position();
    CaseClause* case_clause;
    Next();

    if (!ConsumeOrRecovery<Token::TK_LPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    bool failed = false;
    Expression* expr = ParseExpression(true, res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      if (token_ != Token::TK_RPAREN) {
        SkipUntilSemicolonOrLineTerminator();
      }
      *res = true;  // recovery
      failed = true;
      expr = MakeFailedExpression();
    }

    if (!ConsumeOrRecovery<Token::TK_RPAREN>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    CaseClauses* clauses = factory_->template NewVector<CaseClause*>();
    Target target(this, Target::kSwitchStatement);

    if (!ConsumeOrRecovery<Token::TK_LBRACE>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }

    bool default_found = false;
    while (token_ != Token::TK_RBRACE && token_ != Token::TK_EOS) {
      if (token_ == Token::TK_CASE ||
          token_ == Token::TK_DEFAULT) {
        case_clause = ParseCaseClause(res);
        if (!*res) {
          // case clause recovery
          reporter_->ReportSyntaxError(errors_.back(), begin);
          SkipUntil(Token::TK_RBRACE);
          if (token_ == Token::TK_RBRACE) {
            Next();
          }
          *res = true;  // recovery
          SwitchStatement* const stmt =
              factory_->NewSwitchStatement(expr, clauses,
                                           begin,
                                           lexer_->previous_end_position());
          stmt->set_is_failed_node(true);
          return stmt;
        }
      } else {
        // skip until }
        ReportAndRecoveryUnexpectedToken(token_, begin);
        SkipUntil(Token::TK_RBRACE);
        if (token_ == Token::TK_RBRACE) {
          Next();
        }
        SwitchStatement* const stmt =
            factory_->NewSwitchStatement(expr, clauses,
                                         begin,
                                         lexer_->previous_end_position());
        stmt->set_is_failed_node(true);
        return stmt;
      }
      bool invalid_case_clause = false;
      if (case_clause->IsDefault()) {
        if (default_found) {
          ReportAndRecovery("duplicate default clause in switch", begin);
          failed = true;
          invalid_case_clause = true;
        } else {
          default_found = true;
        }
      }
      if (!invalid_case_clause) {
        clauses->push_back(case_clause);
      }
    }
    if (token_ == Token::TK_EOS) {
      failed = true;
      ReportAndRecoveryUnexpectedToken(token_, begin);
      SwitchStatement* const stmt =
          factory_->NewSwitchStatement(expr, clauses,
                                       begin,
                                       lexer_->previous_end_position());
      stmt->set_is_failed_node(true);
      return stmt;
    }
    Next();
    assert(expr && clauses);
    SwitchStatement* const switch_stmt =
        factory_->NewSwitchStatement(expr, clauses,
                                     begin,
                                     lexer_->previous_end_position());
    switch_stmt->set_is_failed_node(failed);
    target.set_node(switch_stmt);
    return switch_stmt;
  }

//  CaseClauses
//    : CaseClause
//    | CaseClauses CaseClause
//
//  CaseClause
//    : CASE Expression ':' StatementList_opt
//
//  DefaultClause
//    : DEFAULT ':' StatementList_opt
  CaseClause* ParseCaseClause(bool *res) {
    assert(token_ == Token::TK_CASE || token_ == Token::TK_DEFAULT);
    const std::size_t begin = lexer_->begin_position();
    Expression* expr = NULL;
    Statements* const body = factory_->template NewVector<Statement*>();

    if (token_ == Token::TK_CASE) {
      Next();
      expr = ParseExpression(true, CHECK);
    } else  {
      EXPECT(Token::TK_DEFAULT);
    }

    EXPECT(Token::TK_COLON);

    while (token_ != Token::TK_RBRACE &&
           token_ != Token::TK_CASE   &&
           token_ != Token::TK_DEFAULT &&
           token_ != Token::TK_EOS) {
      Statement* const stmt = ParseStatement(res);
      body->push_back(stmt);
    }
    const bool failed = token_ == Token::TK_EOS;

    assert(body);
    CaseClause* clause = factory_->NewCaseClause(expr == NULL,
                                                 expr, body,
                                                 begin, lexer_->previous_end_position());
    clause->set_is_failed_node(failed);
    if (failed) {
      ReportAndRecoveryUnexpectedToken(token_, begin);
    }
    return clause;
  }

//  ThrowStatement
//    : THROW Expression ';'
  Statement* ParseThrowStatement(bool *res) {
    assert(token_ == Token::TK_THROW);
    const std::size_t begin = lexer_->begin_position();
    Next();
    // Throw requires Expression
    if (lexer_->has_line_terminator_before_next()) {
      ReportAndRecovery("missing expression between throw and newline", begin);
      return ReturnFailedStatement(begin);
    }
    Expression* const expr = ParseExpression(true, res);
    if (!*res) {
      // expr is invalid
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    ExpectSemicolon(res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      Statement* stmt = factory_->NewThrowStatement(expr, begin, lexer_->previous_end_position());
      stmt->set_is_failed_node(true);
      return stmt;
    }
    assert(expr);
    return factory_->NewThrowStatement(expr,
                                       begin, lexer_->previous_end_position());
  }

// TryStatement
//    : TRY Block Catch
//    | TRY Block Finally
//    | TRY Block Catch Finally
//
//  Catch
//    : CATCH '(' IDENTIFIER ')' Block
//
//  Finally
//    : FINALLY Block
  Statement* ParseTryStatement(bool *res) {
    assert(token_ == Token::TK_TRY);
    const std::size_t begin = lexer_->begin_position();
    Block* catch_block = NULL;
    Assigned* name = NULL;
    Block* finally_block = NULL;
    bool has_catch_or_finally = false;

    Next();

    std::shared_ptr<jsdoc::Info> info = GetAndResetJSDocInfo();

    if (!CheckOrRecovery<Token::TK_LBRACE>()) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      return ReturnFailedStatement(begin);
    }
    // not consume TK_LBRACE
    Block* const try_block = ParseBlock(res);

    if (token_ == Token::TK_CATCH) {
      // Catch
      has_catch_or_finally = true;
      Next();

      if (!ConsumeOrRecovery<Token::TK_LPAREN>()) {
        reporter_->ReportSyntaxError(errors_.back(), begin);
        return ReturnFailedStatement(begin);
      }

      bool failed = false;
      if (token_ != Token::TK_IDENTIFIER) {
        // if invalid WithStatement is like,
        // with () {  <= error occurred, but token RPAREN is found.
        // }
        // through this error and parse WithStatement body
        if (!CheckOrRecovery<Token::TK_RPAREN>()) {
          *res = true;  // recovery
          failed = true;
        }
        // TODO(Constellation) add precise symbol
      } else {
        iv::core::ast::SymbolHolder sym = ParseSymbol();
        // section 12.14.1
        // within the strict code, Identifier must not be "eval" or "arguments"
        if (strict_) {
          const EvalOrArguments val = IsEvalOrArguments(sym);
          if (val) {
            if (val == kEval) {
              ReportAndRecovery(
                  "catch placeholder \"eval\" not allowed in strict code",
                  begin);
            } else {
              assert(val == kArguments);
              ReportAndRecovery(
                  "catch placeholder \"arguments\" not allowed in strict code",
                  begin);
            }
            failed = true;
          }
        }
        name = factory_->NewAssigned(sym);
      }
      if (!CheckOrRecovery<Token::TK_RPAREN>()) {
        reporter_->ReportSyntaxError(errors_.back(), begin);
        return ReturnFailedStatement(begin);
      }
      Next();
      if (!CheckOrRecovery<Token::TK_LBRACE>()) {
        reporter_->ReportSyntaxError(errors_.back(), begin);
        return ReturnFailedStatement(begin);
      }
      catch_block = ParseBlock(res);
    }

    if (token_ == Token::TK_FINALLY) {
      // Finally
      has_catch_or_finally= true;
      Next();
      if (!CheckOrRecovery<Token::TK_LBRACE>()) {
        reporter_->ReportSyntaxError(errors_.back(), begin);
        return ReturnFailedStatement(begin);
      }
      finally_block = ParseBlock(res);
    }

    if (!has_catch_or_finally) {
      ReportAndRecovery(
          "missing catch or finally after try statement", begin);
      return ReturnFailedStatement(begin);
    }

    assert(try_block);
    Statement* const stmt = factory_->NewTryStatement(try_block,
                                                      name, catch_block,
                                                      finally_block, begin);
    stmt->set_is_failed_node(false);
    if (info) {
      ctx_->Tag(stmt, info);
    }
    return stmt;
  }

//  DebuggerStatement
//    : DEBUGGER ';'
  Statement* ParseDebuggerStatement(bool *res) {
    assert(token_ == Token::TK_DEBUGGER);
    const std::size_t begin = lexer_->begin_position();
    Next();
    ExpectSemicolon(res);
    if (!*res) {
      // recovery
      //
      // debugger TOKEN    ....;  <= SKIP UNTIL THIS
      //
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      Statement* stmt = factory_->NewDebuggerStatement(begin, lexer_->previous_end_position());
      stmt->set_is_failed_node(true);
      return stmt;
    } else {
      return  factory_->NewDebuggerStatement(begin,
                                             lexer_->previous_end_position());
    }
  }

  Statement* ParseExpressionStatement(bool *res) {
    const std::size_t begin = lexer_->begin_position();
    Expression* const expr = ParseExpression(true, res);
    if (!*res) {
      // expr is invalid
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    ExpectSemicolon(res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    assert(expr);
    return factory_->NewExpressionStatement(expr,
                                            lexer_->previous_end_position());
  }

//  LabelledStatement
//    : IDENTIFIER ':' Statement
//
//  ExpressionStatement
//    : Expression ';'
  Statement* ParseExpressionOrLabelledStatement(bool *res) {
    assert(token_ == Token::TK_IDENTIFIER);
    const std::size_t begin = lexer_->begin_position();
    if (lexer_->NextIsColon()) {
      // LabelledStatement
      Symbols* labels = labels_;
      const iv::core::ast::SymbolHolder label = ParseSymbol();
      Next();
      const bool exist_labels = labels;
      if (!exist_labels) {
        labels = factory_->template NewVector<Symbol>();
      }
      bool failed = false;
      if (ContainsLabel(labels, label) || TargetsContainsLabel(label)) {
        // duplicate label
        ReportAndRecovery("duplicate label", begin);
        failed = true;
      } else {
        labels->push_back(label);
      }
      const LabelSwitcher label_switcher(this, labels, exist_labels);

      Statement* const stmt = ParseStatement(res);
      assert(stmt);
      Statement* const l = factory_->NewLabelledStatement(label, stmt);
      l->set_is_failed_node(failed);
      return l;
    }
    Expression* const expr = ParseExpression(true, res);
    if (!*res) {
      // expr is invalid
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    ExpectSemicolon(res);
    if (!*res) {
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      Statement* stmt = factory_->NewExpressionStatement(expr, lexer_->previous_end_position());
      stmt->set_is_failed_node(true);
      return stmt;
    }
    assert(expr);
    return factory_->NewExpressionStatement(expr,
                                            lexer_->previous_end_position());
  }

  Statement* ParseFunctionStatement(bool *res) {
    bool failed = false;
    assert(token_ == Token::TK_FUNCTION);
    const std::size_t begin = lexer_->begin_position();
    if (strict_) {
      ReportAndRecovery(
          "function statement not allowed in strict code",
          lexer_->begin_position());
      failed = true;
    }
    FunctionLiteral* const expr = ParseFunctionLiteral(
        FunctionLiteral::STATEMENT,
        FunctionLiteral::GENERAL,
        res);
    if (!*res) {
      // TODO(Constellation) searching } is better ?
      reporter_->ReportSyntaxError(errors_.back(), begin);
      SkipUntilSemicolonOrLineTerminator();
      *res = true;  // recovery
      return ReturnFailedStatement(begin);
    }
    // define named function as variable declaration
    assert(expr);
    assert(expr->name());
    scope_->AddUnresolved(expr->name().Address(), false);
    Statement* stmt = factory_->NewFunctionStatement(expr);
    stmt->set_is_failed_node(failed);
    return stmt;
  }

//  Expression
//    : AssignmentExpression
//    | Expression ',' AssignmentExpression
  Expression* ParseExpression(bool contains_in, bool *res) {
    Expression* right;
    Expression* result = ParseAssignmentExpression(contains_in, CHECK);
    while (token_ == Token::TK_COMMA) {
      Next();
      right = ParseAssignmentExpression(contains_in, CHECK);
      assert(result && right);
      result = factory_->NewBinaryOperation(Token::TK_COMMA, result, right);
    }
    return result;
  }

//  AssignmentExpression
//    : ConditionalExpression
//    | LeftHandSideExpression AssignmentOperator AssignmentExpression
  Expression* ParseAssignmentExpression(bool contains_in, bool *res) {
    Expression* const result = ParseConditionalExpression(contains_in, CHECK);
    if (!Token::IsAssignOp(token_)) {
      // such as:
      //
      //   /** @const */
      //   Test.prototype.test;
      // or
      //   /** @const */
      //   test;
      //
      if (token_ == Token::TK_SEMICOLON &&
          (result->AsIdentifierAccess() || result->AsIdentifier())) {
        if (std::shared_ptr<jsdoc::Info> info = GetAndResetJSDocInfo()) {
          ctx_->Tag(result, info);
        }
      }
      return result;
    }
    if (!result->IsValidLeftHandSide()) {
      RAISE("invalid left-hand-side in assignment");
    }
    // section 11.13.1 throwing SyntaxError
    if (strict_ && result->AsIdentifier()) {
      const EvalOrArguments val =
          IsEvalOrArguments(result->AsIdentifier()->symbol());
      if (val) {
        if (val == kEval) {
          RAISE("assignment to \"eval\" not allowed in strict code");
        } else {
          assert(val == kArguments);
          RAISE("assignment to \"arguments\" not allowed in strict code");
        }
      }
    }
    const Token::Type op = token_;
    std::shared_ptr<jsdoc::Info> info = GetAndResetJSDocInfo();
    Next();
    Expression* const right = ParseAssignmentExpression(contains_in, CHECK);
    assert(result && right);
    Assignment* assign = factory_->NewAssignment(op, result, right);
    if (info) {
      ctx_->Tag(assign, info);
    }
    return assign;
  }

//  ConditionalExpression
//    : LogicalOrExpression
//    | LogicalOrExpression '?' AssignmentExpression ':' AssignmentExpression
  Expression* ParseConditionalExpression(bool contains_in, bool *res) {
    Expression* result = ParseBinaryExpression(contains_in, 9, CHECK);
    if (token_ == Token::TK_CONDITIONAL) {
      Next();
      // see ECMA-262 section 11.12
      Expression* const left = ParseAssignmentExpression(true, CHECK);
      EXPECT(Token::TK_COLON);
      Expression* const right = ParseAssignmentExpression(contains_in, CHECK);
      assert(result && left && right);
      result = factory_->NewConditionalExpression(result, left, right);
    }
    return result;
  }

//  LogicalOrExpression
//    : LogicalAndExpression
//    | LogicalOrExpression LOGICAL_OR LogicalAndExpression
//
//  LogicalAndExpression
//    : BitwiseOrExpression
//    | LogicalAndExpression LOGICAL_AND BitwiseOrExpression
//
//  BitwiseOrExpression
//    : BitwiseXorExpression
//    | BitwiseOrExpression '|' BitwiseXorExpression
//
//  BitwiseXorExpression
//    : BitwiseAndExpression
//    | BitwiseXorExpression '^' BitwiseAndExpression
//
//  BitwiseAndExpression
//    : EqualityExpression
//    | BitwiseAndExpression '&' EqualityExpression
//
//  EqualityExpression
//    : RelationalExpression
//    | EqualityExpression EQ_STRICT RelationalExpression
//    | EqualityExpression NE_STRICT RelationalExpression
//    | EqualityExpression EQ RelationalExpression
//    | EqualityExpression NE RelationalExpression
//
//  RelationalExpression
//    : ShiftExpression
//    | RelationalExpression LT ShiftExpression
//    | RelationalExpression GT ShiftExpression
//    | RelationalExpression LTE ShiftExpression
//    | RelationalExpression GTE ShiftExpression
//    | RelationalExpression INSTANCEOF ShiftExpression
//    | RelationalExpression IN ShiftExpression
//
//  ShiftExpression
//    : AdditiveExpression
//    | ShiftExpression SHL AdditiveExpression
//    | ShiftExpression SAR AdditiveExpression
//    | ShiftExpression SHR AdditiveExpression
//
//  AdditiveExpression
//    : MultiplicativeExpression
//    | AdditiveExpression ADD MultiplicativeExpression
//    | AdditiveExpression SUB MultiplicativeExpression
//
//  MultiplicativeExpression
//    : UnaryExpression
//    | MultiplicativeExpression MUL UnaryExpression
//    | MultiplicativeExpression DIV UnaryExpression
//    | MultiplicativeExpression MOD UnaryExpression
  Expression* ParseBinaryExpression(bool contains_in, int prec, bool *res) {
    Expression *left, *right;
    Token::Type op;
    left = ParseUnaryExpression(CHECK);
    // MultiplicativeExpression
    while (token_ == Token::TK_MUL ||
           token_ == Token::TK_DIV ||
           token_ == Token::TK_MOD) {
      op = token_;
      Next();
      right = ParseUnaryExpression(CHECK);
      left = ReduceBinaryOperation(op, left, right);
    }
    if (prec < 1) return left;

    // AdditiveExpression
    while (token_ == Token::TK_ADD ||
           token_ == Token::TK_SUB) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 0, CHECK);
      left = ReduceBinaryOperation(op, left, right);
    }
    if (prec < 2) return left;

    // ShiftExpression
    while (token_ == Token::TK_SHL ||
           token_ == Token::TK_SAR ||
           token_ == Token::TK_SHR) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 1, CHECK);
      left = ReduceBinaryOperation(op, left, right);
    }
    if (prec < 3) return left;

    // RelationalExpression
    while ((Token::TK_REL_FIRST < token_ &&
            token_ < Token::TK_REL_LAST) ||
           (contains_in && token_ == Token::TK_IN)) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 2, CHECK);
      assert(left && right);
      left = factory_->NewBinaryOperation(op, left, right);
    }
    if (prec < 4) return left;

    // EqualityExpression
    while (token_ == Token::TK_EQ_STRICT ||
           token_ == Token::TK_NE_STRICT ||
           token_ == Token::TK_EQ ||
           token_ == Token::TK_NE) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 3, CHECK);
      assert(left && right);
      left = factory_->NewBinaryOperation(op, left, right);
    }
    if (prec < 5) return left;

    // BitwiseAndExpression
    while (token_ == Token::TK_BIT_AND) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 4, CHECK);
      left = ReduceBinaryOperation(op, left, right);
    }
    if (prec < 6) return left;

    // BitwiseXorExpression
    while (token_ == Token::TK_BIT_XOR) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 5, CHECK);
      left = ReduceBinaryOperation(op, left, right);
    }
    if (prec < 7) return left;

    // BitwiseOrExpression
    while (token_ == Token::TK_BIT_OR) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 6, CHECK);
      left = ReduceBinaryOperation(op, left, right);
    }
    if (prec < 8) return left;

    // LogicalAndExpression
    while (token_ == Token::TK_LOGICAL_AND) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 7, CHECK);
      assert(left && right);
      left = factory_->NewBinaryOperation(op, left, right);
    }
    if (prec < 9) return left;

    // LogicalOrExpression
    while (token_ == Token::TK_LOGICAL_OR) {
      op = token_;
      Next();
      right = ParseBinaryExpression(contains_in, 8, CHECK);
      assert(left && right);
      left = factory_->NewBinaryOperation(op, left, right);
    }
    return left;
  }

  Expression* ReduceBinaryOperation(Token::Type op,
                                    Expression* left, Expression* right) {
    assert(left && right);
    return factory_->NewBinaryOperation(op, left, right);
  }

//  UnaryExpression
//    : PostfixExpression
//    | DELETE UnaryExpression
//    | VOID UnaryExpression
//    | TYPEOF UnaryExpression
//    | INC UnaryExpression
//    | DEC UnaryExpression
//    | '+' UnaryExpression
//    | '-' UnaryExpression
//    | '~' UnaryExpression
//    | '!' UnaryExpression
  Expression* ParseUnaryExpression(bool *res) {
    Expression *result, *expr;
    const Token::Type op = token_;
    const std::size_t begin = lexer_->begin_position();
    switch (token_) {
      case Token::TK_VOID:
      case Token::TK_NOT:
      case Token::TK_TYPEOF:
        Next();
        expr = ParseUnaryExpression(CHECK);
        assert(expr);
        result = factory_->NewUnaryOperation(op, expr, begin);
        break;

      case Token::TK_DELETE:
        // a strict mode restriction in sec 11.4.1
        // raise SyntaxError when target is direct reference to a variable,
        // function argument, or function name
        Next();
        expr = ParseUnaryExpression(CHECK);
        if (strict_ && expr->AsIdentifier()) {
          RAISE("delete to direct identifier not allowed in strict code");
        }
        assert(expr);
        result = factory_->NewUnaryOperation(op, expr, begin);
        break;

      case Token::TK_BIT_NOT:
        Next();
        expr = ParseUnaryExpression(CHECK);
        result = factory_->NewUnaryOperation(op, expr, begin);
        break;

      case Token::TK_ADD:
        Next();
        expr = ParseUnaryExpression(CHECK);
        if (expr->AsNumberLiteral()) {
          result = expr;
        } else {
          assert(expr);
          result = factory_->NewUnaryOperation(op, expr, begin);
        }
        break;

      case Token::TK_SUB:
        Next();
        expr = ParseUnaryExpression(CHECK);
        result = factory_->NewUnaryOperation(op, expr, begin);
        break;

      case Token::TK_INC:
      case Token::TK_DEC:
        Next();
        expr = ParseMemberExpression(true, CHECK);
        if (!expr->IsValidLeftHandSide()) {
          RAISE("invalid left-hand-side in prefix expression");
        }
        // section 11.4.4, 11.4.5 throwing SyntaxError
        if (strict_ && expr->AsIdentifier()) {
          const EvalOrArguments val =
              IsEvalOrArguments(expr->AsIdentifier()->symbol());
          if (val) {
            if (val == kEval) {
              RAISE("prefix expression to \"eval\" "
                    "not allowed in strict code");
            } else {
              assert(val == kArguments);
              RAISE("prefix expression to \"arguments\" "
                    "not allowed in strict code");
            }
          }
        }
        assert(expr);
        result = factory_->NewUnaryOperation(op, expr, begin);
        break;

      default:
        result = ParsePostfixExpression(CHECK);
        break;
    }
    return result;
  }

//  PostfixExpression
//    : LeftHandSideExpression
//    | LeftHandSideExpression INCREMENT
//    | LeftHandSideExpression DECREMENT
  Expression* ParsePostfixExpression(bool *res) {
    Expression* expr = ParseMemberExpression(true, CHECK);
    if (!lexer_->has_line_terminator_before_next() &&
        (token_ == Token::TK_INC || token_ == Token::TK_DEC)) {
      if (!expr->IsValidLeftHandSide()) {
        RAISE("invalid left-hand-side in postfix expression");
      }
      // section 11.3.1, 11.3.2 throwing SyntaxError
      if (strict_ && expr->AsIdentifier()) {
        const EvalOrArguments val =
            IsEvalOrArguments(expr->AsIdentifier()->symbol());
        if (val) {
          if (val == kEval) {
            RAISE("postfix expression to \"eval\" not allowed in strict code");
          } else {
            assert(val == kArguments);
            RAISE("postfix expression to \"arguments\" "
                  "not allowed in strict code");
          }
        }
      }
      assert(expr);
      expr = factory_->NewPostfixExpression(token_, expr,
                                            lexer_->end_position());
      Next();
    }
    return expr;
  }

//  LeftHandSideExpression
//    : NewExpression
//    | CallExpression
//
//  NewExpression
//    : MemberExpression
//    | NEW NewExpression
//
//  MemberExpression
//    : PrimaryExpression
//    | FunctionExpression
//    | MemberExpression '[' Expression ']'
//    | NEW MemberExpression Arguments
  Expression* ParseMemberExpression(bool allow_call, bool *res) {
    Expression* expr;
    if (token_ != Token::TK_NEW) {
      if (token_ == Token::TK_FUNCTION) {
        // FunctionExpression
        expr = ParseFunctionLiteral(FunctionLiteral::EXPRESSION,
                                    FunctionLiteral::GENERAL, CHECK);
      } else {
        expr = ParsePrimaryExpression(CHECK);
      }
    } else {
      Next();
      Expression* const target = ParseMemberExpression(false, CHECK);
      Expressions* const args = factory_->template NewVector<Expression*>();
      if (token_ == Token::TK_LPAREN) {
        ParseArguments(args, CHECK);
      }
      assert(target && args);
      expr = factory_->NewConstructorCall(target, args, lexer_->previous_end_position());
    }
    while (true) {
      switch (token_) {
        case Token::TK_LBRACK: {
          Next();
          Expression* const index = ParseExpression(true, CHECK);
          assert(expr && index);
          expr = factory_->NewIndexAccess(expr, index);
          EXPECT(Token::TK_RBRACK);
          break;
        }

        case Token::TK_PERIOD: {
          Next<iv::core::IgnoreReservedWords>();  // IDENTIFIERNAME
          // completion hook
          if (IsCompletionPoint()) {
            if (completer_) {
              // this expression is invalid, so shut up this expression
              completer_->RegisterPropertyCompletion(expr);
              UNEXPECT(token_);
            }
          }
          IS(Token::TK_IDENTIFIER);
          const iv::core::ast::SymbolHolder ident = ParseSymbol();
          assert(expr);
          expr = factory_->NewIdentifierAccess(expr, ident);
          break;
        }

        case Token::TK_LPAREN:
          if (allow_call) {
            Expressions* const args =
                factory_->template NewVector<Expression*>();
            ParseArguments(args, CHECK);
            assert(expr && args);
            expr = factory_->NewFunctionCall(expr, args,
                                             lexer_->previous_end_position());
          } else {
            return expr;
          }
          break;

        default:
          return expr;
      }
    }
  }

//  PrimaryExpression
//    : THIS
//    | IDENTIFIER
//    | Literal
//    | ArrayLiteral
//    | ObjectLiteral
//    | '(' Expression ')'
//
//  Literal
//    : NULL_LITERAL
//    | TRUE_LITERAL
//    | FALSE_LITERAL
//    | NUMBER
//    | STRING
//    | REGEXP
  Expression* ParsePrimaryExpression(bool *res) {
    Expression* result = NULL;
    switch (token_) {
      case Token::TK_THIS:
        result = factory_->NewThisLiteral(lexer_->begin_position(),
                                          lexer_->end_position());
        Next();
        break;

      case Token::TK_IDENTIFIER:
        result = ParseIdentifier();
        break;

      case Token::TK_NULL_LITERAL:
        result = factory_->NewNullLiteral(lexer_->begin_position(),
                                          lexer_->end_position());
        Next();
        break;

      case Token::TK_TRUE_LITERAL:
        result = factory_->NewTrueLiteral(lexer_->begin_position(),
                                          lexer_->end_position());
        Next();
        break;

      case Token::TK_FALSE_LITERAL:
        result = factory_->NewFalseLiteral(lexer_->begin_position(),
                                           lexer_->end_position());
        Next();
        break;

      case Token::TK_NUMBER:
        // section 7.8.3
        // strict mode forbids Octal Digits Literal
        if (strict_ && lexer_->NumericType() == lexer_type::OCTAL) {
          RAISE("octal integer literal not allowed in strict code");
        }
        result = factory_->NewNumberLiteral(lexer_->Numeric(),
                                            lexer_->begin_position(),
                                            lexer_->end_position());
        Next();
        break;

      case Token::TK_STRING: {
        const typename lexer_type::State state = lexer_->StringEscapeType();
        if (strict_ && state == lexer_type::OCTAL) {
          RAISE("octal escape sequence not allowed in strict code");
        }
        result = factory_->NewStringLiteral(lexer_->Buffer(),
                                            lexer_->begin_position(),
                                            lexer_->end_position());
        Next();
        break;
      }

      case Token::TK_DIV:
        result = ParseRegExpLiteral(false, CHECK);
        break;

      case Token::TK_ASSIGN_DIV:
        result = ParseRegExpLiteral(true, CHECK);
        break;

      case Token::TK_LBRACK:
        result = ParseArrayLiteral(CHECK);
        break;

      case Token::TK_LBRACE:
        result = ParseObjectLiteral(CHECK);
        break;

      case Token::TK_LPAREN: {
        std::shared_ptr<jsdoc::Info> info = GetAndResetJSDocInfo();
        Next();
        result = ParseExpression(true, CHECK);
        if (!info) {
          info = GetAndResetJSDocInfo();
        }
        if (info && info->HasType()) {
          ctx_->Tag(result, info);
        }
        EXPECT(Token::TK_RPAREN);
        break;
      }

      default:
        UNEXPECT(token_);
        break;
    }
    return result;
  }

//  Arguments
//    : '(' ')'
//    | '(' ArgumentList ')'
//
//  ArgumentList
//    : AssignmentExpression
//    | ArgumentList ',' AssignmentExpression
  template<typename Container>
  Container* ParseArguments(Container* container, bool *res) {
    Next();
    if (token_ != Token::TK_RPAREN) {
      Expression* const first = ParseAssignmentExpression(true, CHECK);
      container->push_back(first);
      while (token_ == Token::TK_COMMA) {
        Next();
        Expression* const expr = ParseAssignmentExpression(true, CHECK);
        container->push_back(expr);
      }
    }
    EXPECT(Token::TK_RPAREN);
    return container;
  }

  Expression* ParseRegExpLiteral(bool contains_eq, bool *res) {
    if (lexer_->ScanRegExpLiteral(contains_eq)) {
      const std::vector<uint16_t> content(lexer_->Buffer());
      if (!lexer_->ScanRegExpFlags()) {
        RAISE("invalid regular expression flag");
      }
      RegExpLiteral* const expr = factory_->NewRegExpLiteral(
          content, lexer_->Buffer(),
          lexer_->begin_position(),
          lexer_->end_position());
      if (!expr) {
        RAISE("invalid regular expression");
      }
      Next();
      return expr;
    } else {
      RAISE("invalid regular expression");
    }
  }

//  ArrayLiteral
//    : '[' Elision_opt ']'
//    | '[' ElementList ']'
//    | '[' ElementList ',' Elision_opt ']'
//
//  ElementList
//    : Elision_opt AssignmentExpression
//    | ElementList ',' Elision_opt AssignmentExpression
//
//  Elision
//    : ','
//    | Elision ','
  Expression* ParseArrayLiteral(bool *res) {
    const std::size_t begin = lexer_->begin_position();
    MaybeExpressions* const items = factory_->template NewVector<iv::core::Maybe<Expression> >();
    Next();
    while (token_ != Token::TK_RBRACK) {
      if (token_ == Token::TK_COMMA) {
        // when Token::TK_COMMA, only increment length
        items->push_back(iv::core::Maybe<Expression>());
      } else {
        Expression* const expr = ParseAssignmentExpression(true, CHECK);
        items->push_back(expr);
      }
      if (token_ != Token::TK_RBRACK) {
        EXPECT(Token::TK_COMMA);
      }
    }
    Next();
    assert(items);
    return factory_->NewArrayLiteral(items,
                                     begin, lexer_->previous_end_position());
  }



//  ObjectLiteral
//    : '{' PropertyNameAndValueList_opt '}'
//
//  PropertyNameAndValueList_opt
//    :
//    | PropertyNameAndValueList
//
//  PropertyNameAndValueList
//    : PropertyAssignment
//    | PropertyNameAndValueList ',' PropertyAssignment
//
//  PropertyAssignment
//    : PropertyName ':' AssignmentExpression
//    | 'get' PropertyName '(' ')' '{' FunctionBody '}'
//    | 'set' PropertyName '(' PropertySetParameterList ')' '{' FunctionBody '}'
//
//  PropertyName
//    : IDENTIFIER
//    | STRING
//    | NUMBER
//
//  PropertySetParameterList
//    : IDENTIFIER
  Expression* ParseObjectLiteral(bool *res) {
    typedef std::unordered_map<Symbol, int> ObjectMap;
    typedef typename ObjectLiteral::Property Property;
    typedef typename ObjectLiteral::Properties Properties;
    const std::size_t begin = lexer_->begin_position();
    Properties* const prop = factory_->template NewVector<Property>();
    ObjectMap map;
    Expression* expr;

    // IDENTIFIERNAME
    Next<iv::core::IgnoreReservedWordsAndIdentifyGetterOrSetter>();
    while (token_ != Token::TK_RBRACE) {
      if (token_ == Token::TK_GET || token_ == Token::TK_SET) {
        const bool is_get = token_ == Token::TK_GET;
        // this is getter or setter or usual prop
        Next<iv::core::IgnoreReservedWords>();  // IDENTIFIERNAME
        if (token_ == Token::TK_COLON) {
          // property
          const iv::core::ast::SymbolHolder ident(
              (is_get) ? iv::core::symbol::get() : iv::core::symbol::set(),
              lexer_->previous_begin_position(),
              lexer_->previous_end_position());
          expr = ParseAssignmentExpression(true, CHECK);
          ObjectLiteral::AddDataProperty(prop, ident, expr);
          typename ObjectMap::iterator it = map.find(ident);
          if (it == map.end()) {
            map.insert(std::make_pair(ident, ObjectLiteral::DATA));
          } else {
            if (it->second != ObjectLiteral::DATA) {
              ReportAndRecovery(
                  "accessor property and data property "
                  "exist with the same name",
                  begin);
            } else {
              if (strict_) {
                ReportAndRecovery(
                    "multiple data property assignments "
                    "with the same name not allowed in strict code", begin);
              }
            }
          }
        } else {
          // getter or setter
          if (token_ == Token::TK_IDENTIFIER ||
              token_ == Token::TK_STRING ||
              token_ == Token::TK_NUMBER) {
            const iv::core::ast::SymbolHolder ident = ParsePropertyName(CHECK);
            typename ObjectLiteral::PropertyDescriptorType type =
                (is_get) ? ObjectLiteral::GET : ObjectLiteral::SET;
            expr = ParseFunctionLiteral(
                FunctionLiteral::EXPRESSION,
                (is_get) ? FunctionLiteral::GETTER : FunctionLiteral::SETTER,
                CHECK);
            ObjectLiteral::AddAccessor(prop, type, ident, expr);
            typename ObjectMap::iterator it = map.find(ident);
            if (it == map.end()) {
              map.insert(std::make_pair(ident, type));
            } else if (it->second & (ObjectLiteral::DATA | type)) {
              if (it->second & ObjectLiteral::DATA) {
                ReportAndRecovery(
                    "data property and accessor property "
                    "exist with the same name", begin);
              } else {
                ReportAndRecovery(
                    "multiple same accessor properties "
                    "exist with the same name", begin);
              }
            } else {
              it->second |= type;
            }
          } else {
            ReportAndRecovery("invalid property name", begin);
          }
        }
      } else if (token_ == Token::TK_IDENTIFIER ||
                 token_ == Token::TK_STRING ||
                 token_ == Token::TK_NUMBER) {
        const iv::core::ast::SymbolHolder ident = ParsePropertyName(CHECK);
        EXPECT(Token::TK_COLON);
        expr = ParseAssignmentExpression(true, CHECK);
        ObjectLiteral::AddDataProperty(prop, ident, expr);
        typename ObjectMap::iterator it = map.find(ident);
        if (it == map.end()) {
          map.insert(std::make_pair(ident, ObjectLiteral::DATA));
        } else {
          if (it->second != ObjectLiteral::DATA) {
            ReportAndRecovery(
                "accessor property and data property "
                "exist with the same name", begin);
          } else {
            if (strict_) {
              ReportAndRecovery(
                  "multiple data property assignments "
                  "with the same name not allowed in strict code", begin);
            }
          }
        }
      } else {
        const std::size_t end = lexer_->begin_position();
        Next();
        ReportAndRecovery("invalid property name", begin);
        return factory_->NewObjectLiteral(prop, begin, end);
      }

      if (token_ != Token::TK_RBRACE) {
        if (!CheckOrRecovery<Token::TK_COMMA>()) {
          // such as,
          //   {
          //     x: "VAL"
          //
          // finish this ObjectLiteral with recovery
          reporter_->ReportSyntaxError(errors_.back(), lexer_->begin_position());
          SkipUntilSemicolonOrLineTerminator();
          assert(prop);
          return factory_->NewObjectLiteral(prop, begin, lexer_->begin_position());
        }
        // IDENTIFIERNAME
        Next<iv::core::IgnoreReservedWordsAndIdentifyGetterOrSetter>();
        if (token_ == Token::TK_RBRACE) {
          // trailing comma found like,
          // var obj = {
          //   test: "OK",
          // };
          // in ES5, this is valid expr, but in ES3, this is not valid.
          // so, report warning
          reporter_->ReportTrailingCommaInObjectLiteral(lexer_->previous_end_position());
        }
      }
    }
    const std::size_t end = lexer_->begin_position();
    Next();
    assert(prop);
    return factory_->NewObjectLiteral(prop, begin, end);
  }

  FunctionLiteral* ParseFunctionLiteral(
      typename FunctionLiteral::DeclType decl_type,
      typename FunctionLiteral::ArgType arg_type,
      bool *res) {
    // IDENTIFIER
    // IDENTIFIER_opt
    std::unordered_set<Symbol> param_set;
    std::size_t throw_error_if_strict_code_line = 0;
    std::size_t throw_error_if_strict_code_number = 0;
    const std::size_t begin_position = lexer_->begin_position();
    enum {
      kDetectNone = 0,
      kDetectEvalName,
      kDetectArgumentsName,
      kDetectEvalParameter,
      kDetectArgumentsParameter,
      kDetectDuplicateParameter,
      kDetectFutureReservedWords
    } throw_error_if_strict_code = kDetectNone;

    Assigneds* const params = factory_->template NewVector<Assigned*>();
    Assigned* name = NULL;

    if (arg_type == FunctionLiteral::GENERAL) {
      assert(token_ == Token::TK_FUNCTION);
      Next(true);  // preparing for strict directive
      const Token::Type current = token_;
      if (current == Token::TK_IDENTIFIER ||
          Token::IsAddedFutureReservedWordInStrictCode(current)) {
        const iv::core::ast::SymbolHolder sym = ParseSymbol();
        if (Token::IsAddedFutureReservedWordInStrictCode(current)) {
          throw_error_if_strict_code = kDetectFutureReservedWords;
          throw_error_if_strict_code_line = lexer_->line_number();
          throw_error_if_strict_code_number = lexer_->previous_end_position();
        } else {
          assert(current == Token::TK_IDENTIFIER);
          const EvalOrArguments val = IsEvalOrArguments(sym);
          if (val) {
            throw_error_if_strict_code = (val == kEval) ?
                kDetectEvalName : kDetectArgumentsName;
            throw_error_if_strict_code_line = lexer_->line_number();
            throw_error_if_strict_code_number = lexer_->previous_end_position();
          }
        }
        name = factory_->NewAssigned(sym);
      } else if (decl_type == FunctionLiteral::DECLARATION ||
                 decl_type == FunctionLiteral::STATEMENT) {
        IS(Token::TK_IDENTIFIER);
      }
    }

    const std::size_t begin_block_position = lexer_->begin_position();

    //  '(' FormalParameterList_opt ')'
    IS(Token::TK_LPAREN);
    Next(true);  // preparing for strict directive

    std::shared_ptr<jsdoc::Info> info = GetAndResetJSDocInfo();

    if (arg_type == FunctionLiteral::GETTER) {
      // if getter, parameter count is 0
      EXPECT(Token::TK_RPAREN);
    } else if (arg_type == FunctionLiteral::SETTER) {
      // if setter, parameter count is 1
      const Token::Type current = token_;
      if (current != Token::TK_IDENTIFIER &&
          !Token::IsAddedFutureReservedWordInStrictCode(current)) {
        IS(Token::TK_IDENTIFIER);
      }
      const iv::core::ast::SymbolHolder ident = ParseSymbol();
      if (!throw_error_if_strict_code) {
        if (Token::IsAddedFutureReservedWordInStrictCode(current)) {
          throw_error_if_strict_code = kDetectFutureReservedWords;
          throw_error_if_strict_code_line = lexer_->line_number();
          throw_error_if_strict_code_number = lexer_->previous_end_position();
        } else {
          assert(current == Token::TK_IDENTIFIER);
          const EvalOrArguments val = IsEvalOrArguments(ident);
          if (val) {
            throw_error_if_strict_code = (val == kEval) ?
                kDetectEvalName : kDetectArgumentsName;
            throw_error_if_strict_code_line = lexer_->line_number();
            throw_error_if_strict_code_number = lexer_->previous_end_position();
          }
        }
      }
      params->push_back(factory_->NewAssigned(ident));
      EXPECT(Token::TK_RPAREN);
    } else {
      if (token_ != Token::TK_RPAREN) {
        do {
          const Token::Type current = token_;
          if (current != Token::TK_IDENTIFIER &&
              !Token::IsAddedFutureReservedWordInStrictCode(current)) {
            IS(Token::TK_IDENTIFIER);
          }
          const iv::core::ast::SymbolHolder ident = ParseSymbol();
          if (!throw_error_if_strict_code) {
            if (Token::IsAddedFutureReservedWordInStrictCode(current)) {
              throw_error_if_strict_code = kDetectFutureReservedWords;
              throw_error_if_strict_code_line = lexer_->line_number();
              throw_error_if_strict_code_number = lexer_->previous_end_position();
            } else {
              assert(current == Token::TK_IDENTIFIER);
              const EvalOrArguments val = IsEvalOrArguments(ident);
              if (val) {
                throw_error_if_strict_code = (val == kEval) ?
                    kDetectEvalName : kDetectArgumentsName;
                throw_error_if_strict_code_line = lexer_->line_number();
                throw_error_if_strict_code_number = lexer_->previous_end_position();
              }
            }
            if ((!throw_error_if_strict_code) &&
                (param_set.find(ident) != param_set.end())) {
              throw_error_if_strict_code = kDetectDuplicateParameter;
              throw_error_if_strict_code_line = lexer_->line_number();
              throw_error_if_strict_code_number = lexer_->previous_end_position();
            }
          }
          params->push_back(factory_->NewAssigned(ident));
          param_set.insert(ident);
          if (token_ == Token::TK_COMMA) {
            Next(true);
          } else {
            break;
          }
        } while (true);
      }
      EXPECT(Token::TK_RPAREN);
    }

    //  '{' FunctionBody '}'
    //
    //  FunctionBody
    //    :
    //    | SourceElements
    EXPECT(Token::TK_LBRACE);

    Statements* const body = factory_->template NewVector<Statement*>();
    Scope* const scope = factory_->NewScope(decl_type);
    const ScopeSwitcher scope_switcher(this, scope);
    const TargetSwitcher target_switcher(this);
    const bool function_is_strict =
        ParseSourceElements(Token::TK_RBRACE, body, CHECK);
    if (strict_ || function_is_strict) {
      // section 13.1
      // Strict Mode Restrictions
      switch (throw_error_if_strict_code) {
        case kDetectNone:
          break;
        case kDetectEvalName:
          ReportAndRecovery(
              "function name \"eval\" not allowed in strict code",
              throw_error_if_strict_code_number,
              throw_error_if_strict_code_line);
          break;
        case kDetectArgumentsName:
          ReportAndRecovery(
              "function name \"arguments\" not allowed in strict code",
              throw_error_if_strict_code_number,
              throw_error_if_strict_code_line);
          break;
        case kDetectEvalParameter:
          ReportAndRecovery(
              "parameter \"eval\" not allowed in strict code",
              throw_error_if_strict_code_number,
              throw_error_if_strict_code_line);
          break;
        case kDetectArgumentsParameter:
          ReportAndRecovery(
              "parameter \"arguments\" not allowed in strict code",
              throw_error_if_strict_code_number,
              throw_error_if_strict_code_line);
          break;
        case kDetectDuplicateParameter:
          ReportAndRecovery(
              "duplicate parameter not allowed in strict code",
              throw_error_if_strict_code_number,
              throw_error_if_strict_code_line);
          break;
        case kDetectFutureReservedWords:
          ReportAndRecovery(
              "FutureReservedWords is found in strict code",
              throw_error_if_strict_code_number,
              throw_error_if_strict_code_line);
          break;
      }
    }
    Next();
    const std::size_t end_block_position = lexer_->previous_end_position();
    assert(params && body && scope);

    FunctionLiteral* function =
        factory_->NewFunctionLiteral(decl_type,
                                     name,
                                     params,
                                     body,
                                     scope,
                                     function_is_strict,
                                     begin_block_position,
                                     end_block_position,
                                     begin_position,
                                     end_block_position);
    // register function literal to upper scope
    scope_->GetUpperScope()->RegisterFunctionLiteral(function);
    if (completer_ &&
        completer_->HasCompletionPoint() &&
        !completer_->HasTargetFunction()) {
      completer_->RegisterTargetFunction(function);
    }
    if (info) {
      ctx_->Tag(function, info);
    }
    return function;
  }

  iv::core::ast::SymbolHolder ParsePropertyName(bool* res) {
    if (token_ == Token::TK_NUMBER) {
      if (strict_ && lexer_->NumericType() == lexer_type::OCTAL) {
        RAISE_WITH(
            "octal integer literal not allowed in strict code",
            iv::core::ast::SymbolHolder());
      }
      const double val = lexer_->Numeric();
      iv::core::dtoa::StringPieceDToA builder;
      builder.Build(val);
      const Symbol name = Intern(builder.buffer());
      Next();
      return iv::core::ast::SymbolHolder(
          name,
          lexer_->previous_begin_position(),
          lexer_->previous_end_position());
    } else if (token_ == Token::TK_STRING) {
      if (strict_ && lexer_->StringEscapeType() == lexer_type::OCTAL) {
        RAISE_WITH(
            "octal escape sequence not allowed in strict code",
            iv::core::ast::SymbolHolder());
      }
      const Symbol name = Intern(lexer_->Buffer());
      Next();
      return iv::core::ast::SymbolHolder(
          name,
          lexer_->previous_begin_position(),
          lexer_->previous_end_position());
    } else {
      return ParseSymbol();
    }
  }

  iv::core::ast::SymbolHolder ParseSymbol() {
    const Symbol ident = Intern(lexer_->Buffer());
    Next();
    return iv::core::ast::SymbolHolder(
        ident,
        lexer_->previous_begin_position(),
        lexer_->previous_end_position());
  }

  Identifier* ParseIdentifier() {
    assert(token_ == Token::TK_IDENTIFIER);
    Identifier* const ident = factory_->NewIdentifier(
        Token::TK_IDENTIFIER,
        Intern(lexer_->Buffer()),
        lexer_->begin_position(),
        lexer_->end_position());
    Next();
    return ident;
  }

  bool ContainsLabel(const Symbols* labels, Symbol label) const {
    return labels &&
        std::find(labels->begin(), labels->end(), label) != labels->end();
  }

  bool TargetsContainsLabel(Symbol label) const {
    for (const Target* target = target_;
         target != NULL;
         target = target->previous()) {
      if (ContainsLabel(target->labels(), label)) {
        return true;
      }
    }
    return false;
  }

  BreakableStatement** LookupBreakableTarget(Symbol label) const {
    for (Target* target = target_;
         target != NULL;
         target = target->previous()) {
      if (ContainsLabel(target->labels(), label)) {
        return target->node();
      }
    }
    return NULL;
  }

  BreakableStatement** LookupBreakableTarget() const {
    for (Target* target = target_;
         target != NULL;
         target = target->previous()) {
      if (target->IsAnonymous()) {
        return target->node();
      }
    }
    return NULL;
  }

  IterationStatement** LookupContinuableTarget(Symbol label) const {
    for (Target* target = target_;
         target != NULL;
         target = target->previous()) {
      if (target->IsIteration() && ContainsLabel(target->labels(), label)) {
        return reinterpret_cast<IterationStatement**>(target->node());
      }
    }
    return NULL;
  }

  IterationStatement** LookupContinuableTarget() const {
    for (Target* target = target_;
         target != NULL;
         target = target->previous()) {
      if (target->IsIteration()) {
        return reinterpret_cast<IterationStatement**>(target->node());
      }
    }
    return NULL;
  }

  void ReportUnexpectedToken(Token::Type expected_token) {
    switch (token_) {
      case Token::TK_STRING:
        error_.append("unexpected string");
        break;
      case Token::TK_NUMBER:
        error_.append("unexpected number");
        break;
      case Token::TK_IDENTIFIER:
        error_.append("unexpected identifier");
        break;
      case Token::TK_EOS:
        error_.append("unexpected EOS");
        break;
      case Token::TK_ILLEGAL: {
        error_.append("illegal character");
        break;
      }
      default: {
        error_.append("unexpected token ");
        error_.append(Token::ToString(token_));
        break;
      }
    }
  }

  bool ExpectSemicolon(bool *res) {
    if (token_ == Token::TK_SEMICOLON) {
      Next();
      return true;
    }
    if (lexer_->has_line_terminator_before_next() ||
        token_ == Token::TK_RBRACE ||
        token_ == Token::TK_EOS ) {
      // automatic semicolon insertion : ASI
      // report this
      reporter_->ReportAutomaticSemicolonInsertion(lexer_->previous_end_position());
      return true;
    }
    UNEXPECT(token_);
  }

  inline lexer_type* lexer() const {
    return lexer_;
  }

  inline Token::Type Peek() const {
    return token_;
  }

  inline Scope* scope() const {
    return scope_;
  }

  inline void set_scope(Scope* scope) {
    scope_ = scope;
  }

  inline const std::string& error() const {
    return error_;
  }

  inline const std::vector<std::string>& errors() const {
    return errors_;
  }

  inline Target* target() const {
    return target_;
  }

  inline void set_target(Target* target) {
    target_ = target;
  }

  inline Symbols* labels() const {
    return labels_;
  }

  inline void set_labels(Symbols* labels) {
    labels_ = labels;
  }

  inline bool strict() const {
    return strict_;
  }

  inline void set_strict(bool strict) {
    strict_ = strict;
  }

  inline bool RecoverableError() const {
    return (!(error_state_ & kNotRecoverable)) && token_ == Token::TK_EOS;
  }

  inline AstFactory* factory() const {
    return factory_;
  }

 protected:
  class ScopeSwitcher : private iv::core::Noncopyable<> {
   public:
    ScopeSwitcher(parser_type* parser, Scope* scope)
      : parser_(parser) {
      scope->SetUpperScope(parser_->scope());
      parser_->set_scope(scope);
    }
    ~ScopeSwitcher() {
      assert(parser_->scope() != NULL);
      parser_->set_scope(parser_->scope()->GetUpperScope());
    }
   private:
    parser_type* parser_;
  };

  class LabelSwitcher : private iv::core::Noncopyable<> {
   public:
    LabelSwitcher(parser_type* parser,
                  Symbols* labels, bool exist_labels)
      : parser_(parser),
        exist_labels_(exist_labels) {
      parser_->set_labels(labels);
    }
    ~LabelSwitcher() {
      if (!exist_labels_) {
        parser_->set_labels(NULL);
      }
    }
   private:
    parser_type* parser_;
    bool exist_labels_;
  };

  class StrictSwitcher : private iv::core::Noncopyable<> {
   public:
    explicit StrictSwitcher(parser_type* parser)
      : parser_(parser),
        prev_(parser->strict()) {
    }
    ~StrictSwitcher() {
      parser_->set_strict(prev_);
    }
    inline void SwitchStrictMode() const {
      parser_->set_strict(true);
    }
    inline bool IsStrict() const {
      return parser_->strict();
    }
   private:
    parser_type* parser_;
    bool prev_;
  };

  enum EvalOrArguments {
    kNone = 0,
    kEval = 1,
    kArguments = 2
  };

  static EvalOrArguments IsEvalOrArguments(Symbol sym) {
    if (sym == iv::core::symbol::eval()) {
      return kEval;
    } else if (sym == iv::core::symbol::arguments()) {
      return kArguments;
    } else {
      return kNone;
    }
  }

  template<iv::core::Token::Type token>
  bool ConsumeOrRecovery() {
    if (CheckOrRecovery<token>()) {
      Next();
      return true;
    }
    return false;
  }

  template<iv::core::Token::Type token>
  bool CheckOrRecovery() {
    if (token_ != token) {
      ReportUnexpectedToken(token);
      errors_.push_back(error_);
      error_.clear();
      SkipUntilSemicolonOrLineTerminator();
      return false;
    }
    return true;
  }

  template<typename LexType>
  inline Token::Type Next() {
    bool completion = false;
    while (true) {
      token_ = lexer_->Next<LexType>(strict_);
      if (lexer_->IsCompletionPoint()) {
        completion = true;
      }
      if (token_ == Token::TK_SINGLE_LINE_COMMENT ||
          token_ == Token::TK_MULTI_LINE_COMMENT) {
        HandleComment(token_);
      } else {
        assert(token_ != Token::TK_SINGLE_LINE_COMMENT &&
               token_ != Token::TK_MULTI_LINE_COMMENT);
        completion_point_ = completion;
        return token_;
      }
    }
  }

  inline Token::Type Next() {
    return Next<iv::core::IdentifyReservedWords>();
  }

  inline Token::Type Next(bool strict) {
    bool completion = false;
    while (true) {
      token_ = lexer_->Next<iv::core::IdentifyReservedWords>(strict);
      if (lexer_->IsCompletionPoint()) {
        completion = true;
      }
      if (token_ == Token::TK_SINGLE_LINE_COMMENT ||
          token_ == Token::TK_MULTI_LINE_COMMENT) {
        HandleComment(token_);
      } else {
        assert(token_ != Token::TK_SINGLE_LINE_COMMENT &&
               token_ != Token::TK_MULTI_LINE_COMMENT);
        completion_point_ = completion;
        return token_;
      }
    }
  }

  void SkipComment(Token::Type token) {
    token_ = token;
    bool completion = lexer_->IsCompletionPoint();
    while (true) {
      if (token_ == Token::TK_SINGLE_LINE_COMMENT ||
          token_ == Token::TK_MULTI_LINE_COMMENT) {
        HandleComment(token_);
      } else {
        assert(token_ != Token::TK_SINGLE_LINE_COMMENT &&
               token_ != Token::TK_MULTI_LINE_COMMENT);
        completion_point_ = completion;
        return;
      }
      token_ = lexer_->Next<iv::core::IdentifyReservedWords>(strict_);
      if (lexer_->IsCompletionPoint()) {
        completion = true;
      }
    }
  }

  void SkipUntilSemicolonOrLineTerminator() {
    Skip skip(lexer_, strict_);
    SkipComment(skip.SkipUntilSemicolonOrLineTerminator());
  }

  void SkipUntil(Token::Type token) {
    Skip skip(lexer_, strict_);
    SkipComment(skip.SkipUntil(token));
  }

  void HandleComment(Token::Type token) {
    const std::size_t begin = lexer_->begin_position();
    const std::size_t end = lexer_->end_position();
    const iv::core::UStringPiece comment =
        structured_.original().substr(begin, end - begin + 1);
    if (comment.size() > 5 &&  // target comment size is more than "/***/"
        token == Token::TK_MULTI_LINE_COMMENT) {
      if (comment[0] == '/' && comment[1] == '*' && comment[2] == '*') {
        // this is JSDoc start mark
        // so, parse JSDoc
        DebugLog("PARSING JSDoc");
        jsdoc::Provider provider(factory_);
        provider.Parse(comment);
        doc_ = provider.GetInfo();
      }
    }
  }

  Statement* ReturnFailedStatement(std::size_t begin) {
    Statement* stmt = factory_->NewEmptyStatement(begin, lexer_->previous_end_position());
    stmt->set_is_failed_node(true);
    return stmt;
  }

  Expression* MakeFailedExpression() {
    // create dummy expr
    // expr is not valid, but, only parse Statement phase
    // FIXME(Constellation)
    // using true literal instead. but, we should use error expr
    return factory_->NewTrueLiteral(0, 0);
  }

  std::shared_ptr<jsdoc::Info> GetAndResetJSDocInfo() {
    std::shared_ptr<jsdoc::Info> doc = doc_;
    doc_.reset();
    return doc;
  }

  void ReportAndRecovery(const char* str, std::size_t pos) {
    error_state_ |= kNotRecoverable;
    error_.append(str);
    errors_.push_back(error_);
    error_.clear();
    reporter_->ReportSyntaxError(errors_.back(), pos);
  }

  void ReportAndRecovery(const char* str, std::size_t pos, std::size_t line) {
    // not used line number in this recovery parser
    ReportAndRecovery(str, pos);
  }

  void ReportAndRecoveryUnexpectedToken(Token::Type token, std::size_t pos) {
    ReportUnexpectedToken(token);
    errors_.push_back(error_);
    error_.clear();
    reporter_->ReportSyntaxError(errors_.back(), pos);
  }

  bool IsCompletionPoint() const {
    return completion_point_;
  }

  Context* ctx_;
  lexer_type* lexer_;
  Token::Type token_;
  bool completion_point_;
  std::string error_;
  std::vector<std::string> errors_;
  bool strict_;
  int error_state_;
  AstFactory* factory_;
  Reporter* reporter_;
  Completer* completer_;
  const StructuredSource& structured_;
  Scope* scope_;
  Target* target_;
  Symbols* labels_;
  std::shared_ptr<jsdoc::Info> doc_;
};
#undef IS
#undef IS_NORETURN
#undef EXPECT
#undef UNEXPECT
#undef RAISE
#undef RAISE_WITH_NUMBER
#undef RAISE_RECOVERVABLE
#undef CHECK
}  // namespace az
#endif  // AZ_PARSER_H_
