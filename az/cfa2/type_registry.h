#ifndef AZ_CFA2_TYPE_REGISTRY_H_
#define AZ_CFA2_TYPE_REGISTRY_H_
#include <iv/detail/unordered_map.h>
#include <az/debug_log.h>
#include <az/ast_fwd.h>
#include <az/jsdoc/type_ast_fwd.h>
namespace az {
namespace cfa2 {

class TypeRegistry {
 public:
  TypeRegistry()
    : named_map_(),
      param_tagged_() {
  }

  void RegisterAssignedType(Expression* lhs, FunctionLiteral* literal) {
    iv::core::UString name;
    assert(lhs->IsValidLeftHandSide());
    // get name from Assignment
    if (TypeRegistry::NormalizeName(lhs, &name)) {
      DebugLog(name);
      named_map_.insert(std::make_pair(name, literal));
    }
  }

  void RegisterNamedType(FunctionLiteral* literal) {
    // if FunctionLiteral name found, use this
    if (iv::core::Maybe<Assigned> a = literal->name()) {
      named_map_.insert(
          std::make_pair(
              iv::core::symbol::GetSymbolString(a.Address()->symbol()),
              literal));
    }
  }

  FunctionLiteral* GetRegisteredConstructorOrInterface(const iv::core::UStringPiece& piece) {
    std::unordered_map<iv::core::UString, FunctionLiteral*>::const_iterator it = named_map_.find(piece);
    if (it != named_map_.end()) {
      return it->second;
    }
    return NULL;
  }

  void RegisterFunctionLiteralWithParamType(FunctionLiteral* literal,
                                            jsdoc::TypeExpression* param) {
    param_tagged_.insert(std::make_pair(literal, param));
  }

 private:
  static bool NormalizeName(Expression* lhs, iv::core::UString* name) {
    std::vector<uint16_t> reversed;
    Expression* current = lhs;
    while (true) {
      if (Identifier* ident = current->AsIdentifier()) {
        name->assign(iv::core::symbol::GetSymbolString(ident->symbol()));
        name->insert(name->end(), reversed.rbegin(), reversed.rend());
        return true;
      } else if (IdentifierAccess* access = current->AsIdentifierAccess()) {
        const iv::core::UString str(
            iv::core::symbol::GetSymbolString(access->key()));
        reversed.insert(
            reversed.end(),
            str.begin(), str.end());
        reversed.push_back('.');
        current = access->target();
      } else {
        return false;
      }
    }
    return true;  // make compiler happy
  }

  std::unordered_map<iv::core::UString, FunctionLiteral*> named_map_;
  std::unordered_multimap<FunctionLiteral*, jsdoc::TypeExpression*> param_tagged_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_TYPE_REGISTRY_H_
