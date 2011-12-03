// symbol
// the original logic is derived from iv / lv5 symbol implementation
#ifndef AZ_SYMBOL_H_
#define AZ_SYMBOL_H_
#include <cstdio>
#include <iv/detail/cstdint.h>
#include <iv/detail/cinttypes.h>
#include <iv/detail/unordered_set.h>
#include <iv/detail/functional.h>
#include <iv/detail/array.h>
#include <iv/platform.h>
#include <iv/byteorder.h>
#include <iv/conversions.h>
#include <iv/ustring.h>
#include <iv/singleton.h>
#include <iv/thread.h>
#include <iv/symbol.h>
namespace az {

using iv::core::Symbol;

class SymbolTable : public iv::core::Singleton<SymbolTable> {
 private:
  friend class iv::core::Singleton<SymbolTable>;

  SymbolTable()
    : sync_(),
      table_() {
  }

 public:
  template<class CharT>
  inline Symbol Intern(const CharT* str) {
    iv::core::thread::ScopedLock<iv::core::thread::Mutex> lock(&sync_);
    return table_.Lookup(str);
  }

  template<class String>
  inline Symbol Intern(const String& str) {
    iv::core::thread::ScopedLock<iv::core::thread::Mutex> lock(&sync_);
    return table_.Lookup(str);
  }

  inline Symbol Intern(uint32_t index) {
    iv::core::thread::ScopedLock<iv::core::thread::Mutex> lock(&sync_);
    return iv::core::symbol::MakeSymbolFromIndex(index);
  }

 private:
  iv::core::thread::Mutex sync_;
  iv::core::SymbolTable table_;
};

template<typename Range>
inline Symbol Intern(const Range& range) {
  return SymbolTable::Instance()->Intern(range);
}

inline Symbol Intern(const iv::core::UStringPiece& piece) {
  return SymbolTable::Instance()->Intern(piece);
}

inline Symbol Intern(const iv::core::StringPiece& piece) {
  return SymbolTable::Instance()->Intern(piece);
}

inline Symbol Intern(uint32_t index) {
  return SymbolTable::Instance()->Intern(index);
}

}  // namespace az
#endif  // AZ_SYMBOL_H_
