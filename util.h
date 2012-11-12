#ifndef PROTODATA_UTIL_H
#define PROTODATA_UTIL_H

#include <sstream>

std::string join(std::ostringstream& stream) {
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

#endif
