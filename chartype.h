#ifndef PROTODATA_CHARTYPE_H
#define PROTODATA_CHARTYPE_H

inline bool is_alphabetic(uint32_t rune) {
  return (rune >= U'A' && rune <= U'Z')
    || (rune >= U'a' && rune <= U'z');
}

inline bool is_numeric(uint32_t rune) {
  return rune >= U'0' && rune <= U'9';
}

inline bool is_alphanumeric(uint32_t rune) {
  return is_alphabetic(rune) || is_numeric(rune);
}

inline bool is_whitespace(uint32_t rune) {
  return rune == U' ' || (rune >= U'\u0009' && rune <= U'\u000d');
}

#endif
