// heuristic js type
#ifndef _AZ_JSTYPE_H_
#define _AZ_JSTYPE_H_
namespace az {

enum JSType {
  // initial value
  TYPE_NOT_SEARCHED = 0,

  // Any Type
  TYPE_ANY,

  // Primitive JSTypes
  TYPE_STRING,
  TYPE_NUMBER,
  TYPE_BOOLEAN,
  TYPE_UNDEFINED,
  TYPE_NULL,

  // Literally Analyzable JSTypes
  TYPE_FUNCTION,
  TYPE_REGEXP,
  TYPE_ARRAY,
  TYPE_OBJECT,
  TYPE_USER_OBJECT
};

}  // namespace az
#endif  // _AZ_JSTYPE_H_
