#ifndef PROTODATA_CHARTYPE_H
#define PROTODATA_CHARTYPE_H

typedef bool chartype_function(uint32_t);
chartype_function
  is_alphabetic,
  is_alphanumeric,
  is_binary,
  is_decimal,
  is_hexadecimal,
  is_octal,
  is_whitespace;

inline bool is_alphabetic(const uint32_t rune) {
  return (rune >= U'A' && rune <= U'Z')
    || (rune >= U'a' && rune <= U'z');
}

inline bool is_alphanumeric(const uint32_t rune) {
  return is_alphabetic(rune) || is_decimal(rune);
}

inline bool is_binary(const uint32_t rune) {
  return rune == U'0' || rune == U'1';
}

inline bool is_decimal(const uint32_t rune) {
  return rune >= U'0' && rune <= U'9';
}

inline bool is_hexadecimal(const uint32_t rune) {
  return is_decimal(rune)
    || (rune >= U'A' && rune <= U'F')
    || (rune >= U'a' && rune <= U'f');
}

inline bool is_octal(const uint32_t rune) {
  return rune >= U'0' && rune <= U'7';
}

inline bool is_whitespace(const uint32_t rune) {
  return rune == U' ' || (rune >= U'\u0009' && rune <= U'\u000d');
}

#endif
