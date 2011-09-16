#ifndef AZ_CFA2_TYPE_INTERPRETER_H_
#define AZ_CFA2_TYPE_INTERPRETER_H_
#include <az/cfa2/type_interpreter_fwd.h>
namespace az {
namespace cfa2 {


void TypeInterpreter::Visit(jsdoc::PrefixQuestionExpression* node) {
  node->expr()->Accept(this);
  result_.Join(AVAL_NULL);
}

void TypeInterpreter::Visit(jsdoc::PrefixBangExpression* node) {
  node->expr()->Accept(this);
  result_.ExcludeBase(AVAL_NULL);
}

void TypeInterpreter::Visit(jsdoc::PostfixQuestionExpression* node) {
  node->expr()->Accept(this);
  result_.Join(AVAL_NULL);
}

void TypeInterpreter::Visit(jsdoc::PostfixBangExpression* node) {
  node->expr()->Accept(this);
  result_.ExcludeBase(AVAL_NULL);
}

void TypeInterpreter::Visit(jsdoc::QuestionLiteral* node) {
  result_.Reset(AVAL_NOBASE);
}

void TypeInterpreter::Visit(jsdoc::StarLiteral* node) {
  result_.Reset(AVAL_NOBASE);
}

void TypeInterpreter::Visit(jsdoc::NullLiteral* node) {
  result_.Reset(AVAL_NULL);
}

void TypeInterpreter::Visit(jsdoc::UndefinedLiteral* node) {
  result_.Reset(AVAL_UNDEFINED);
}

void TypeInterpreter::Visit(jsdoc::VoidLiteral* node) {
  result_.Reset(AVAL_UNDEFINED);
}

void TypeInterpreter::Visit(jsdoc::UnionType* node) {
  AVal res(AVAL_NOBASE);
  jsdoc::TypeExpressions* exprs = node->exprs();
  assert(!exprs->empty());
  for (jsdoc::TypeExpressions::const_iterator it = exprs->begin(),
       last = exprs->end(); it != last; ++it) {
    (*it)->Accept(this);
    res.Join(result_);
  }
  result_ = res;
}

void TypeInterpreter::Visit(jsdoc::ArrayType* node) {
  AObject* ary = heap_->GetDeclObject(node);
  if (ary) {
    // already created, so use it
    result_.Reset(ary);
    return;
  }
  ary = heap_->GetFactory()->NewAObject();
  heap_->DeclObject(node, ary);
  const std::vector<AVal> args;
  ARRAY_CONSTRUCTOR(heap_, AVal(ary), args, true);

  assert(ary);
  uint32_t index = 0;
  for (jsdoc::TypeExpressions::const_iterator it = node->exprs()->begin(),
       last = node->exprs()->end(); it != last; ++it, ++index) {
    (*it)->Accept(this);
    ary->UpdateProperty(heap_, Intern(index), result_);
  }
  result_.Reset(ary);
}

void TypeInterpreter::Visit(jsdoc::RecordType* node) {
  AObject* obj = heap_->GetDeclObject(node);
  if (obj) {
    // already created, so use it
    result_.Reset(obj);
    return;
  }
  obj = heap_->MakeObject();
  heap_->DeclObject(node, obj);

  assert(obj);
  for (jsdoc::FieldTypes::const_iterator it = node->exprs()->begin(),
       last = node->exprs()->end(); it != last; ++it) {
    jsdoc::FieldType* field = *it;
    if (field->HasValue()) {
      field->value()->Accept(this);
      obj->UpdateProperty(heap_, Intern(*field->key()), result_);
    } else {
      obj->UpdateProperty(heap_, Intern(*field->key()), AVal(AVAL_NOBASE));
    }
  }
  result_.Reset(obj);
}

void TypeInterpreter::Visit(jsdoc::FieldType* node) {
  // TODO(Constellation) implement it
  UNREACHABLE();
}

void TypeInterpreter::Visit(jsdoc::FunctionType* node) {
  // TODO(Constellation) implement it
  result_.Reset();
}

void TypeInterpreter::Visit(jsdoc::NameExpression* node) {
  const jsdoc::NameString* str = node->value();
  // primitive type check phase
  if (IsEqualIgnoreCase(*str, "string")) {
    result_.Reset(AVAL_STRING);
  } else if (IsEqualIgnoreCase(*str, "number")) {
    result_.Reset(AVAL_NUMBER);
  } else if (IsEqualIgnoreCase(*str, "boolean")) {
    result_.Reset(AVAL_BOOL);
  } else if (lookuped_.find(*node->value()) == lookuped_.end()) {
    // class lookup
    // see type registry
    lookuped_.insert(*node->value());
    if (FunctionLiteral* literal = heap_->registry()->GetRegisteredConstructorOrInterface(*node->value())) {
      const std::vector<AVal> args;
      AObject* this_binding = heap_->GetDeclObject(node);
      if (!this_binding) {
        this_binding = heap_->MakeObject();
        heap_->DeclObject(node, this_binding);
      }
      AObject* target = heap_->GetDeclObject(literal);
      this_binding->UpdatePrototype(heap_,
                                    target->GetProperty(Intern("prototype")));
      // jsdoc @extends / @interface check
      // TODO(Constellation) lookup only prototype and set
      if (std::shared_ptr<jsdoc::Info> info = heap_->GetInfo(literal)) {
        if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_IMPLEMENTS)) {
          tag->type()->Accept(this);
          this_binding->UpdatePrototype(heap_, result_);
        }
        if (std::shared_ptr<jsdoc::Tag> tag = info->GetTag(jsdoc::Token::TK_EXTENDS)) {
          tag->type()->Accept(this);
          this_binding->UpdatePrototype(heap_, result_);
        }
      }
      // not call FunctionLiteral
      // because FunctionLiteral parses TypeExpression @return, so may recur
      result_.Reset(this_binding);
    } else {
      // search target in current scope
      // TODO(Constellation) implement it
      result_.Reset(AVAL_NOBASE);
    }
  } else {
    // type recursion like,
    //   /**
    //    * @constructor
    //    * @extends {SubTest}
    //    */
    //   function Test() { }
    //
    //   /**
    //    * @constructor
    //    * @extends {Test}
    //    */
    //   function SubTest() { }
    result_.Reset(AVAL_NOBASE);
  }
}

void TypeInterpreter::Visit(jsdoc::TypeNameWithApplication* node) {
  // TODO:(Constellation) use application data
  node->expr()->Accept(this);
}

void TypeInterpreter::Visit(jsdoc::ParametersType* node) {
  UNREACHABLE();
}

void TypeInterpreter::Visit(jsdoc::RestExpression* node) {
  UNREACHABLE();
}

void TypeInterpreter::Visit(jsdoc::PostfixEqualExpression* node) {
  UNREACHABLE();
}

} }  // namespace az::cfa2
#endif  // AZ_CFA2_TYPE_INTERPRETER_H_
