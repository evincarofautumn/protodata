#ifndef PROTODATA_TOKENIZER_H
#define PROTODATA_TOKENIZER_H

#include <iterator>
#include <stdexcept>

#include <rx/ignorer.h>
#include <rx/utf8_output_encoder.h>

#include <Token.h>
#include <chartype.h>
#include <util.h>

template<class R>
class tokenizer_t {
public:
  tokenizer_t(tokenizer_t&& other)
    : input(std::move(other.input)), current(other.current) {}
  tokenizer_t(R&& range) : input(std::move(range)) { advance(); }
  bool empty() const { return input.empty(); }
  void pop_front() { advance(); }
  Token front() const {
    return current;
  }
  tokenizer_t() = delete;
  tokenizer_t(const tokenizer_t&) = delete;
  tokenizer_t& operator=(tokenizer_t&&) = delete;
  tokenizer_t& operator=(const tokenizer_t&) = delete;
private:
  void advance() {
    using namespace rx;
    using namespace std;
    string token;
    ignorer ignore;
    auto output(utf8_output_encoder(output_iterator(back_inserter(token))));
    while (accept_if(is_whitespace, input, ignore)) {}
    if (accept(0x7b, input, ignore)) {
      current = Token::push();
    } else if (accept(0x7d, input, ignore)) {
      current = Token::pop();
    } else if (accept_if(is_alphabetic, input, output)) {
      while (accept_if(is_alphanumeric, input, output)) {}
      throw runtime_error(join("Unknown keyword: \"", token, "\"."));
    } else if (!input.empty()) {
      throw runtime_error(join("Invalid character '", input.front(), "'."));
    } else {
      throw runtime_error("Unexpected end of input.");
    }
  }
  R input;
  Token current;
};

template<class R>
tokenizer_t<R> tokenizer(R&& range) {
  return tokenizer_t<R>(std::move(range));
}

#endif
