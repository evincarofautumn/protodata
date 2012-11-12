#ifndef UTF8_INPUT_DECODER_H
#define UTF8_INPUT_DECODER_H
#include <cstdint>
#include <utf8.h>
#include <vector>

namespace rx {

template<class R>
class utf8_input_decoder_t {
public:
  utf8_input_decoder_t(R&& range) : input(std::move(range)) { advance(); }
  bool empty() const { return input.empty(); }
  void pop_front() { advance(); }
  uint32_t front() const { return current; }
private:
  // Read octets until a valid UTF-8 sequence appears or
  // no more octets are available.
  void advance() {
    using namespace utf8::internal;
    while (!input.empty()) {
      buffer.push_back(input.front());
      input.pop_front();
      auto boundary = buffer.begin();
      const auto error = validate_next(boundary, buffer.end(), current);
      if (error == utf8::internal::UTF8_OK) {
        buffer.erase(buffer.begin(), boundary);
        break;
      } else if (buffer.size() >= max_utf8_size) {
	throw std::runtime_error("Invalid UTF-8 in input.");
      }
    }
  }
  static const int max_utf8_size = 4;
  R input;
  std::vector<char> buffer;
  uint32_t current;
};

template<class R>
utf8_input_decoder_t<R> utf8_input_decoder(R&& range) {
  return utf8_input_decoder_t<R>(std::move(range));
}

}

#endif
