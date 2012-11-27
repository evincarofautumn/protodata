#ifndef PROTODATA_TOKEN_H
#define PROTODATA_TOKEN_H

#include <iomanip>
#include <iostream>

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
  enum Type {
    NOOP,
    PUSH,
    POP,
    WRITE_SIGNED,
    WRITE_UNSIGNED,
    WRITE_DOUBLE,
    SET_ENDIANNESS,
    SET_SIGNEDNESS,
    SET_WIDTH,
    SET_FORMAT,
  };
  enum Width {
    WIDTH_8 = 8,
    WIDTH_16 = 16,
    WIDTH_32 = 32,
    WIDTH_64 = 64,
  };
  typedef int64_t Signed;
  typedef uint64_t Unsigned;
  typedef double Double;
  Term();
  Term(Endianness);
  Term(Signedness);
  Term(Width);
  Term(Format);
  static Term push();
  static Term pop();
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
  };
  Type type;
  Value value;
private:
  Term(Type, void*);
};

#endif
