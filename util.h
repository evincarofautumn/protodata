#ifndef PROTODATA_UTIL_H
#define PROTODATA_UTIL_H

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

#endif
