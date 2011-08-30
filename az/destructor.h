#ifndef AZ_DESTRUCTOR_H_
#define AZ_DESTRUCTOR_H_
namespace az {

struct Destructor {
  typedef void result_type;

  template<typename T>
  result_type operator()(T* ptr) const {
    ptr->~T();
  }
};

template<typename T>
struct TypedDestructor {
  typedef void result_type;
  template<typename U>
  result_type operator()(U* ptr) const {
    static_cast<T*>(ptr)->~T();
  }
};

}  // namespace az
#endif  // AZ_DESTRUCTOR_H_
