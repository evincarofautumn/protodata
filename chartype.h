#ifndef PROTODATA_CHARTYPE_H
#define PROTODATA_CHARTYPE_H

inline bool is_alphabetic(uint32_t rune) {
  return (rune >= U'A' && rune <= U'Z')
    || (rune >= U'a' && rune <= U'z');
}

inline bool is_binary(uint32_t rune) {
  return rune == U'0' || rune == U'1';
}

inline bool is_octal(uint32_t rune) {
  return rune >= U'0' && rune <= U'7';
}

inline bool is_decimal(uint32_t rune) {
  return rune >= U'0' && rune <= U'9';
}

inline bool is_hexadecimal(uint32_t rune) {
  return is_decimal(rune)
    || (rune >= U'A' && rune <= U'F')
    || (rune >= U'a' && rune <= U'f');
}

inline bool is_alphanumeric(uint32_t rune) {
  return is_alphabetic(rune) || is_decimal(rune);
}

inline bool is_whitespace(uint32_t rune) {
  return rune == U' ' || (rune >= U'\u0009' && rune <= U'\u000d');
}

#endif
