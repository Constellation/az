// CFA2 interpreter
#ifndef AZ_CFA2_INTERPRETER_FWD_H_
#define AZ_CFA2_INTERPRETER_FWD_H_
#include <vector>
#include <deque>
#include <exception>
#include <iv/detail/cstdint.h>
#include <iv/noncopyable.h>
#include <az/ast_fwd.h>
#include <az/jstype.h>
#include <az/cfa2/heap.h>
#include <az/cfa2/binding.h>
#include <az/cfa2/result.h>
#include <az/jsdoc/type_ast.h>
#include <az/jsdoc/type_ast_visitor.h>
namespace az {
namespace cfa2 {

class Interpreter
  : private iv::core::Noncopyable<Interpreter>,
    public MutableAstVisitor,
    public jsdoc::TypeAstVisitor {
 public:
  typedef std::deque<Statement*> Tasks;
  typedef std::deque<std::pair<Statement*, Result> > Errors;
  typedef std::vector<Binding*> Bindings;

  class UnwindStack : public std::exception {
   public:
    UnwindStack(FunctionLiteral* literal,
                const AVal& this_binding,
                const std::vector<AVal>& args)
      : std::exception(),
        literal_(literal),
        this_binding_(this_binding),
        args_(args) { }
    virtual ~UnwindStack() throw() { }

    FunctionLiteral* literal() const {
      return literal_;
    }

    const AVal& this_binding() const {
      return this_binding_;
    }

    const std::vector<AVal>& args() const {
      return args_;
    }

   private:
    FunctionLiteral* literal_;
    const AVal this_binding_;
    const std::vector<AVal> args_;
  };

  explicit Interpreter(Heap* heap)
    : heap_(heap),
      result_(AVal(AVAL_NOBASE)),
      base_(AVAL_NOBASE),
      frame_(NULL) {
  }

  inline void Run(FunctionLiteral* global);

  inline Result EvaluateFunction(AObject* function,
                                 const AVal& this_binding,
                                 const std::vector<AVal>& args,
                                 bool IsConstructorCalled);

  Frame* CurrentFrame() const {
    return frame_;
  }

  void set_current_frame(Frame* frame) {
    frame_ = frame;
  }

  Heap* heap() const {
    return heap_;
  }

  const Result& result() const {
    return result_;
  }

 private:
  // JS AST Visitor
  inline void Visit(Block* block);
  inline void Visit(FunctionStatement* func);
  inline void Visit(FunctionDeclaration* func);
  inline void Visit(VariableStatement* var);
  inline void Visit(EmptyStatement* stmt);
  inline void Visit(IfStatement* stmt);
  inline void Visit(DoWhileStatement* stmt);
  inline void Visit(WhileStatement* stmt);
  inline void Visit(ForStatement* stmt);
  inline void Visit(ForInStatement* stmt);
  inline void Visit(ContinueStatement* stmt);
  inline void Visit(BreakStatement* stmt);
  inline void Visit(ReturnStatement* stmt);
  inline void Visit(WithStatement* stmt);
  inline void Visit(LabelledStatement* stmt);
  inline void Visit(SwitchStatement* stmt);
  inline void Visit(ThrowStatement* stmt);
  inline void Visit(TryStatement* stmt);
  inline void Visit(DebuggerStatement* stmt);
  inline void Visit(ExpressionStatement* stmt);

  inline void Visit(Assignment* assign);
  inline void Visit(BinaryOperation* binary);
  inline void Visit(ConditionalExpression* cond);
  inline void Visit(UnaryOperation* unary);
  inline void Visit(PostfixExpression* postfix);
  inline void Visit(StringLiteral* literal);
  inline void Visit(NumberLiteral* literal);
  inline void Visit(Identifier* literal);
  inline void Visit(ThisLiteral* literal);
  inline void Visit(NullLiteral* lit);
  inline void Visit(TrueLiteral* lit);
  inline void Visit(FalseLiteral* lit);
  inline void Visit(RegExpLiteral* literal);
  inline void Visit(ArrayLiteral* literal);
  inline void Visit(ObjectLiteral* literal);
  inline void Visit(FunctionLiteral* literal);
  inline void Visit(IdentifierAccess* prop);
  inline void Visit(IndexAccess* prop);
  inline void Visit(FunctionCall* call);
  inline void Visit(ConstructorCall* call);

  inline void Visit(Declaration* dummy);
  inline void Visit(CaseClause* clause);

  inline void Interpret(FunctionLiteral* literal);

  inline void GainCompletion(Completer* completer);
  inline void EvaluateCompletionTargetFunction(Completer* completer);

  inline Result Assign(Assignment* assign, Result res, AVal old);
  inline Result Assign(Expression* lhs, Result res);

  // JSDoc TypeExpression Visitor
  inline void Visit(jsdoc::PrefixQuestionExpression* node);
  inline void Visit(jsdoc::PrefixBangExpression* node);
  inline void Visit(jsdoc::PostfixQuestionExpression* node);
  inline void Visit(jsdoc::PostfixBangExpression* node);
  inline void Visit(jsdoc::QuestionLiteral* node);
  inline void Visit(jsdoc::StarLiteral* node);
  inline void Visit(jsdoc::NullLiteral* node);
  inline void Visit(jsdoc::UndefinedLiteral* node);
  inline void Visit(jsdoc::VoidLiteral* node);
  inline void Visit(jsdoc::UnionType* node);
  inline void Visit(jsdoc::ArrayType* node);
  inline void Visit(jsdoc::RecordType* node);
  inline void Visit(jsdoc::FieldType* node);
  inline void Visit(jsdoc::FunctionType* node);
  inline void Visit(jsdoc::NameExpression* node);
  inline void Visit(jsdoc::TypeNameWithApplication* node);
  inline void Visit(jsdoc::ParametersType* node);
  inline void Visit(jsdoc::RestExpression* node);
  inline void Visit(jsdoc::PostfixEqualExpression* node);

  Heap* heap_;
  Result result_;  // result value
  AVal base_;  // only use in FunctionCall / ConstructorCall this binding
  Frame* frame_;
  Errors* errors_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_INTERPRETER_FWD_H_
