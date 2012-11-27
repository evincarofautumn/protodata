#ifndef PROTODATA_UTIL_H
#define PROTODATA_UTIL_H

#include <utf8.h>

#include <sstream>
#include <stdexcept>

inline std::string join(std::ostringstream& stream) {
  return stream.str();
}

template<class T, class... Ts>
std::string join(std::ostringstream& stream, const T& x, Ts... xs) {
  stream << x;
  return join(stream, xs...);
}

template<class... Ts>
std::string join(const Ts&... xs) {
  std::ostringstream stream;
  return join(stream, xs...);
}

#define IMPOSSIBLE(MESSAGE) \
  throw std::logic_error(join("Impossible error: ", (MESSAGE), "."))

namespace utf8 {
  template<class u16_iterator>
  u16_iterator append16(uint32_t rune, u16_iterator result) {
    if (rune > 0xffff) {
      *result++ = static_cast<uint16_t>((rune >> 10) + internal::LEAD_OFFSET);
      *result++ = static_cast<uint16_t>
        ((rune & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
    } else {
      *result++ = static_cast<uint16_t>(rune);
    }
    return result;
  }
}

template<class T>
const char* serialize_cast(const T& x) {
  return reinterpret_cast<const char*>(x);
}

#endif
