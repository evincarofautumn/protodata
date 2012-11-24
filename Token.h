#ifndef PROTODATA_TOKEN_H
#define PROTODATA_TOKEN_H
#include <iomanip>
#include <iostream>

class Token {
public:
  Token() : type(WRITE) { value.as_unsigned = 0; }
  static Token push() {
    Token token;
    token.type = PUSH;
    token.value.as_unsigned = 0;
    return token;
  }
  static Token pop() {
    Token token;
    token.type = POP;
    token.value.as_unsigned = 0;
    return token;
  }
  static Token write(uint64_t value) {
    Token token;
    token.type = WRITE;
    token.value.as_unsigned = value;
    return token;
  }
  static Token write(int64_t value) {
    Token token;
    token.type = WRITE;
    token.value.as_signed = value;
    return token;
  }
  static Token write(double value) {
    Token token;
    token.type = WRITE;
    token.value.as_double = value;
    return token;
  }
  friend std::ostream& operator<<(std::ostream& stream, const Token& token) {
    std::ios state(0);
    state.copyfmt(stream);
    switch (token.type) {
    case PUSH:
      stream << "push";
      break;
    case POP:
      stream << "pop";
      break;
    case WRITE:
      stream << "write(0x" << std::hex << std::setw(16)
        << std::setfill('0') << token.value.as_unsigned << ")";
      break;
    }
    stream.copyfmt(state);
    return stream;
  }
private:
  enum Type {
    PUSH,
    POP,
    WRITE,
  } type;
  union Value {
    int64_t  as_signed;
    uint64_t as_unsigned;
    double   as_double;
  } value;
};

#endif
