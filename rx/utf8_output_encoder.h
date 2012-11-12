#ifndef UTF8_OUTPUT_DECODER_H
#define UTF8_OUTPUT_DECODER_H
#include <vector>
#include <rx/container.h>
#include <utf8.h>

namespace rx {

template<class R>
class utf8_output_encoder_t {
public:
  explicit utf8_output_encoder_t(R&& range) : output(std::move(range)) {}
  utf8_output_encoder_t(utf8_output_encoder_t&& other)
    : output(std::move(other.output)) {}
  void put(uint32_t rune) {
    using namespace std;
    using namespace utf8::unchecked;
    vector<uint8_t> decoded;
    append(rune, back_inserter(decoded));
    copy(container(decoded), output);
  }
  utf8_output_encoder_t() = delete;
  utf8_output_encoder_t(const utf8_output_encoder_t&) = delete;
  utf8_output_encoder_t& operator=(utf8_output_encoder_t&&) = delete;
  utf8_output_encoder_t& operator=(const utf8_output_encoder_t&) = delete;
private:
  R output;
};

template<class R>
utf8_output_encoder_t<typename std::remove_reference<R>::type>
  utf8_output_encoder(R&& range) {
  return utf8_output_encoder_t<typename std::remove_reference<R>::type>
    (std::move(range));
}

}

#endif
