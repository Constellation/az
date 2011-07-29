#ifndef _AZ_DELETER_H_
#define _AZ_DELETER_H_
namespace az {

struct Deleter {
  typedef void result_type;

  template<typename T>
  result_type operator()(T* ptr) const {
    delete ptr;
  }
};

}  // namespace az
#endif  // _AZ_DELETER_H_
