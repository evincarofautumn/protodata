#ifndef PROTODATA_CHARTYPE_H
#define PROTODATA_CHARTYPE_H

inline bool is_alphabetic(uint32_t rune) {
  return (rune >= 0x0041 && rune <= 0x005a)
    || (rune >= 0x0061 && rune <= 0x007a);
}

inline bool is_numeric(uint32_t rune) {
  return rune >= 0x0030 && rune <= 0x0039;
}

inline bool is_alphanumeric(uint32_t rune) {
  return is_alphabetic(rune) || is_numeric(rune);
}

inline bool is_whitespace(uint32_t rune) {
  return rune == 0x0020 || (rune >= 0x0009 && rune <= 0x000d);
}

#endif
