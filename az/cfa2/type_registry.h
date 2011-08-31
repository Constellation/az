#ifndef AZ_CFA2_TYPE_REGISTRY_H_
#define AZ_CFA2_TYPE_REGISTRY_H_
#include <az/debug_log.h>
#include <az/ast_fwd.h>
namespace az {
namespace cfa2 {

class TypeRegistry {
 public:
  TypeRegistry()
    : map_() {
  }

  void RegisterAssignedType(Expression* lhs, FunctionLiteral* literal) {
    iv::core::UString name;
    assert(lhs->IsValidLeftHandSide());
    // if FunctionLiteral name found, use this
    if (iv::core::Maybe<Identifier> i = literal->name()) {
      Identifier* ident = i.Address();
      name.assign(ident->value().begin(), ident->value().end());
      map_.insert(std::make_pair(name, literal));
    } else {
      // get name from Assignment
      if (TypeRegistry::NormalizeName(lhs, &name)) {
        DebugLog(name);
        map_.insert(std::make_pair(name, literal));
      }
    }
  }

 private:
  static bool NormalizeName(Expression* lhs, iv::core::UString* name) {
    // TODO:(Constellation) fix it more efficiently
    std::vector<uint16_t> reversed;
    Expression* current = lhs;
    while (true) {
      if (Identifier* ident = current->AsIdentifier()) {
        name->append(ident->value().begin(), ident->value().end());
        name->insert(name->end(), reversed.rbegin(), reversed.rend());
        return true;
      } else if (IdentifierAccess* access = current->AsIdentifierAccess()) {
        Identifier* prop = access->key();
        reversed.insert(reversed.end(),
                        prop->value().rbegin(), prop->value().rend());
        reversed.push_back('.');
        current = access->target();
      } else {
        return false;
      }
    }
    return true;  // make compiler happy
  }

  std::unordered_map<iv::core::UString, FunctionLiteral*> map_;
};

} }  // namespace az::cfa2
#endif  // AZ_CFA2_TYPE_REGISTRY_H_
