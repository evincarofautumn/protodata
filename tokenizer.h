#ifndef PROTODATA_TOKENIZER_H
#define PROTODATA_TOKENIZER_H

#include <iterator>
#include <deque>
#include <stdexcept>

#include <rx/ignorer.h>
#include <rx/utf8_output_encoder.h>

#include <Token.h>
#include <chartype.h>
#include <util.h>

template<class R>
class tokenizer_t {
public:
  tokenizer_t(tokenizer_t&&) = default;
  tokenizer_t(R&& range) : input(std::move(range)) { advance(); }
  bool empty() const { return input.empty() && buffer.empty(); }
  void pop_front() { buffer.pop_front(); advance(); }
  Token front() const { return buffer.front(); }
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
    if (input.empty()) return;
    if (accept(U'{', input, ignore)) {
      buffer.push_back(Token::push());
    } else if (accept(U'}', input, ignore)) {
      buffer.push_back(Token::pop());
    } else if (accept(U'0', input, ignore)) {
      if (accept(U'b', input, ignore)) {
        while (accept_if(is_binary, input, output)
          || accept(U'_', input, ignore)) {}
        buffer.push_back(write_signed(token, 2));
      } else if (accept(U'o', input, ignore)) {
        while (accept_if(is_octal, input, output)
          || accept(U'_', input, ignore)) {}
        buffer.push_back(write_signed(token, 8));
      } else if (accept(U'x', input, ignore)) {
        while (accept_if(is_hexadecimal, input, output)
          || accept(U'_', input, ignore)) {}
        buffer.push_back(write_signed(token, 16));
      } else if (accept(U'.', input, output)) {
        while (accept_if(is_decimal, input, output)
          || accept(U'_', input, ignore)) {}
        buffer.push_back(write_double(token));
      } else {
        while (accept_if(is_decimal, input, output)
          || accept(U'_', input, ignore)) {}
        buffer.push_back(write_signed(token, 10));
      }
    } else if (accept_if(is_decimal, input, output)) {
      while (accept_if(is_decimal, input, output)
        || accept(U'_', input, ignore)) {}
      if (accept(U'.', input, output)) {
        while (accept_if(is_decimal, input, output)
          || accept(U'_', input, ignore)) {}
        buffer.push_back(write_double(token));
      } else {
        buffer.push_back(write_signed(token, 10));
      }
    } else if (accept_if(is_alphabetic, input, output)) {
      while (accept_if(is_alphanumeric, input, output)) {}
      throw runtime_error(join("Unknown keyword: \"", token, "\"."));
    } else {
      throw runtime_error(join("Invalid character '", input.front(), "'."));
    }
  }
  Token write_signed(const std::string& token, int base) {
    char* boundary;
    const auto value = std::strtoll(token.c_str(), &boundary, base);
    if (boundary != token.c_str() + token.size())
      throw std::runtime_error(join("Invalid base-", base,
        " signed integer literal: \"", token, "\""));
    return Token::write(int64_t(value));
  }
  Token write_unsigned(const std::string& token, int base) {
    char* boundary;
    const auto value = std::strtoull(token.c_str(), &boundary, base);
    if (boundary != token.c_str() + token.size())
      throw std::runtime_error(join("Invalid base-", base,
        " unsigned integer literal: \"", token, "\""));
    return Token::write(uint64_t(value));
  }
  Token write_double(const std::string& token) {
    char* boundary;
    const auto value = std::strtod(token.c_str(), &boundary);
    if (boundary != token.c_str() + token.size())
      throw std::runtime_error
        (join("Invalid floating-point literal: \"", token, "\""));
    return Token::write(value);
  }
  R input;
  std::deque<Token> buffer;
};

template<class R>
tokenizer_t<R> tokenizer(R&& range) {
  return tokenizer_t<R>(std::move(range));
}

#endif
