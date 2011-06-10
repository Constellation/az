#ifndef _AZ_VARIABLE_TYPE_H_
#define _AZ_VARIABLE_TYPE_H_
namespace az {

enum VariableType {
  VARIABLE_STACK,     // explicitly defined stack  variable
  VARIABLE_GLOBAL,    // explicitly defined global variable
  VARIABLE_HEAP       // implicitly defined global variable
};

}  // namespace az
#endif  // _AZ_VARIABLE_TYPE_H_
