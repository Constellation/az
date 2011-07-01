#ifndef _AZ_SKIP_H_
#define _AZ_SKIP_H_
namespace az {

// Skip class skips heuristic skip sources
template<typename Lexer>
class BasicSkip {
 public:
  explicit BasicSkip(const Lexer& lexer)
    : lexer_(lexer) {
  }

  void SkipUntilSemicolonOrLineTerminator(std::size_t end) {
  }

 private:
  const Lexer& lexer_;
};

}  // namespace az
#endif  // _AZ_SKIP_H_
