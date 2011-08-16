// mark complete tokens
#ifndef _AZ_COMPLETER_H_
#define _AZ_COMPLETER_H_
namespace az {

class Completer {
 public:
  Completer()
    : has_completion_point_(false) {
  }

  void SetCompletionPoint() {
    has_completion_point_ = true;
  }

  bool HasCompletionPoint() const {
    return has_completion_point_;
  }

 private:
  bool has_completion_point_;
};

}  // namespace az
#endif  // _AZ_COMPLETER_H_
