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
namespace az {

template<std::size_t PointerSize, bool IsLittle>
struct SymbolLayout;

static const uint32_t kSymbolIsIndex = 0xFFFFFFFFUL;

template<>
struct SymbolLayout<4, true> {
  typedef SymbolLayout<4, true> this_type;
  union {
    struct {
      uint32_t high_;
      uint32_t low_;
    } index_;
    struct {
      const iv::core::UString* str_;
      uint32_t low_;
    } str_;
    uint64_t bytes_;
  };
};

template<>
struct SymbolLayout<8, true> {
  typedef SymbolLayout<8, true> this_type;
  union {
    struct {
      uint32_t high_;
      uint32_t low_;
    } index_;
    struct {
      const iv::core::UString* str_;
    } str_;
    uint64_t bytes_;
  };
};

template<>
struct SymbolLayout<4, false> {
  typedef SymbolLayout<4, false> this_type;
  union {
    struct {
      uint32_t low_;
      uint32_t high_;
    } index_;
    struct {
      uint32_t low_;
      const iv::core::UString* str_;
    } str_;
    uint64_t bytes_;
  };
};

template<>
struct SymbolLayout<8, false> {
  typedef SymbolLayout<8, false> this_type;
  union {
    struct {
      uint32_t low_;
      uint32_t high_;
    } index_;
    struct {
      const iv::core::UString* str_;
    } str_;
    uint64_t bytes_;
  };
};

typedef SymbolLayout<
  iv::core::Size::kPointerSize,
  iv::core::kLittleEndian> Symbol;

inline bool operator==(Symbol x, Symbol y) {
  return x.bytes_ == y.bytes_;
}

inline bool operator!=(Symbol x, Symbol y) {
  return x.bytes_ != y.bytes_;
}

inline bool operator<(Symbol x, Symbol y) {
  return x.bytes_ < y.bytes_;
}

inline bool operator>(Symbol x, Symbol y) {
  return x.bytes_ > y.bytes_;
}

inline bool operator<=(Symbol x, Symbol y) {
  return x.bytes_ <= y.bytes_;
}

inline bool operator>=(Symbol x, Symbol y) {
  return x.bytes_ >= y.bytes_;
}

struct SymbolStringHolder {
  const iv::core::UString* symbolized_;
  friend bool operator==(SymbolStringHolder x, SymbolStringHolder y) {
    return (*x.symbolized_) == (*y.symbolized_);
  }

  friend bool operator!=(SymbolStringHolder x, SymbolStringHolder y) {
    return (*x.symbolized_) != (*y.symbolized_);
  }

  friend bool operator<(SymbolStringHolder x, SymbolStringHolder y) {
    return (*x.symbolized_) < (*y.symbolized_);
  }

  friend bool operator>(SymbolStringHolder x, SymbolStringHolder y) {
    return (*x.symbolized_) > (*y.symbolized_);
  }

  friend bool operator<=(SymbolStringHolder x, SymbolStringHolder y) {
    return (*x.symbolized_) <= (*y.symbolized_);
  }

  friend bool operator>=(SymbolStringHolder x, SymbolStringHolder y) {
    return (*x.symbolized_) >= (*y.symbolized_);
  }
};

inline bool IsIndexSymbol(Symbol sym) {
  return sym.index_.low_ == kSymbolIsIndex;
}

inline bool IsArrayIndexSymbol(Symbol sym) {
  return (sym.index_.low_ == kSymbolIsIndex) && (sym.index_.high_ < UINT32_MAX);
}

inline bool IsStringSymbol(Symbol sym) {
  return !IsIndexSymbol(sym);
}

inline const iv::core::UString* GetStringFromSymbol(Symbol sym) {
  assert(IsStringSymbol(sym));
  return sym.str_.str_;
}

inline uint32_t GetIndexFromSymbol(Symbol sym) {
  assert(IsIndexSymbol(sym));
  return sym.index_.high_;
}

inline iv::core::UString GetIndexStringFromSymbol(Symbol sym) {
  assert(IsIndexSymbol(sym));
  const uint32_t index = GetIndexFromSymbol(sym);
  std::array<char, 15> buf;
  return iv::core::UString(
      buf.data(),
      buf.data() + snprintf(
          buf.data(), buf.size(), "%"PRIu32,
          index));
}

inline iv::core::UString GetSymbolString(Symbol sym) {
  if (IsIndexSymbol(sym)) {
    return GetIndexStringFromSymbol(sym);
  } else {
    return *GetStringFromSymbol(sym);
  }
}

}  // namespace az
namespace IV_HASH_NAMESPACE_START {

// template specialization for Symbol in std::unordered_map
template<>
struct hash<az::Symbol>
  : public std::unary_function<az::Symbol, std::size_t> {
  inline result_type operator()(const argument_type& x) const {
    return hash<uint64_t>()(x.bytes_);
  }
};

// template specialization for SymbolStringHolder in std::unordered_map
template<>
struct hash<az::SymbolStringHolder>
  : public std::unary_function<az::SymbolStringHolder, std::size_t> {
  inline result_type operator()(const argument_type& x) const {
    return hash<iv::core::UString>()(*x.symbolized_);
  }
};

} IV_HASH_NAMESPACE_END
namespace az {

class SymbolTable : public iv::core::Singleton<SymbolTable> {
 private:
  friend class iv::core::Singleton<SymbolTable>;
  typedef std::unordered_set<SymbolStringHolder> Set;

  SymbolTable()
    : sync_(),
      set_() {
  }

  ~SymbolTable() {
    for (Set::const_iterator it = set_.begin(),
         last = set_.end(); it != last; ++it) {
      delete it->symbolized_;
    }
  }

  // for type dispatching
  static Symbol MakeSymbol(const iv::core::UString* str) {
    Symbol symbol = { { { 0u, 0u } } };
    symbol.str_.str_ = str;
    return symbol;
  }

  static Symbol MakeSymbol(uint32_t index) {
    Symbol symbol;
    symbol.index_.high_ = index;
    symbol.index_.low_ = kSymbolIsIndex;
    return symbol;
  }

 public:
  template<class CharT>
  inline Symbol Intern(const CharT* str) {
    using std::char_traits;
    return Lookup(iv::core::BasicStringPiece<CharT>(str));
  }

  template<class String>
  inline Symbol Intern(const String& str) {
    uint32_t index;
    if (iv::core::ConvertToUInt32(str.begin(), str.end(), &index)) {
      return MakeSymbol(index);
    }
    const iv::core::UString target(str.begin(), str.end());
    SymbolStringHolder holder = { &target };
    {
      iv::core::thread::ScopedLock<iv::core::thread::Mutex> lock(&sync_);
      typename Set::const_iterator it = set_.find(holder);
      if (it != set_.end()) {
        return MakeSymbol(it->symbolized_);
      } else {
        holder.symbolized_ = new iv::core::UString(target);
        set_.insert(holder);
        return MakeSymbol(holder.symbolized_);
      }
    }
  }

  inline Symbol Intern(uint32_t index) {
    return MakeSymbol(index);
  }

 private:

  iv::core::thread::Mutex sync_;
  Set set_;
};

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
