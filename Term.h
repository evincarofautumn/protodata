#ifndef PROTODATA_TOKEN_H
#define PROTODATA_TOKEN_H

#include <cstdint>
#include <string>

class Term {
public:
  enum Endianness {
    NATIVE,
    LITTLE,
    BIG,
  };
  enum Signedness {
    UNSIGNED,
    SIGNED,
  };
  enum Format {
    INTEGER,
    FLOAT,
    UNICODE,
  };
  enum Function {
    FILE,
  };
  enum Type {
    NOOP,
    PUSH,
    POP,
    FUNCTION,
    FUNCTION_CLOSE_BRACKET,
    NEXT_PARAMETER,
    WRITE_SIGNED,
    WRITE_UNSIGNED,
    WRITE_DOUBLE,
    SET_ENDIANNESS,
    SET_SIGNEDNESS,
    SET_WIDTH,
    SET_FORMAT,
  };
  typedef int64_t Signed;
  typedef uint64_t Unsigned;
  typedef double Double;
  typedef unsigned int Width;
  Term();
  Term(Endianness);
  Term(Signedness);
  Term(Width);
  Term(Format);
  Term(Function);
  static Term push();
  static Term pop();
  static Term close_bracket();
  static Term next_parameter();
  static Term write(uint64_t);
  static Term write(int64_t);
  static Term write(double);
  union Value {
    Signed as_signed;
    Unsigned as_unsigned;
    Double as_double;
    Endianness as_endianness;
    Signedness as_signedness;
    Width as_width;
    Format as_format;
    Function as_function;
  };
  Type type;
  Value value;
private:
  Term(Type, const void*);
};

#endif
