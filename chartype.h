#ifndef PROTODATA_CHARTYPE_H
#define PROTODATA_CHARTYPE_H

inline bool is_alphabetic(uint32_t rune) {
  return (rune >= 0x41 && rune <= 0x5a)
    || (rune >= 0x61 && rune <= 0x7a);
}

inline bool is_numeric(uint32_t rune) {
  return rune >= 0x30 && rune <= 0x39;
}

inline bool is_alphanumeric(uint32_t rune) {
  return is_alphabetic(rune) || is_numeric(rune);
}

inline bool is_whitespace(uint32_t rune) {
  return rune == 0x20 || (rune >= 0x09 && rune <= 0x0d);
}

#endif
