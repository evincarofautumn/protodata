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

  tokenizer_t(R&& range) : input(std::move(range)),
    output(rx::output_iterator(std::back_inserter(buffer))) {
    advance(output);
  }

  bool empty() const {
    return input.empty() && buffer.empty();
  }

  void pop_front() {
    buffer.pop_front();
    advance(output);
  }

  Token front() const { return buffer.front(); }

  tokenizer_t() = delete;
  tokenizer_t(const tokenizer_t&) = delete;
  tokenizer_t& operator=(tokenizer_t&&) = delete;
  tokenizer_t& operator=(const tokenizer_t&) = delete;

private:

# define Accept(X) accept(X, input, append)
# define AcceptIf(X) accept_if(X, input, append)
# define Ignore(X) accept(X, input, ignore)
# define IgnoreIf(X) accept_if(X, input, ignore)
# define Many(X) while (X) {}
# define Output(X) output.put(X);

  template<class O>
  void advance(O output) {

    using namespace rx;
    using namespace std;

    string token;
    ignorer ignore;
    auto append(utf8_output_encoder(output_iterator(back_inserter(token))));

    Many(IgnoreIf(is_whitespace) || ignore_comment())
    if (input.empty()) return;

    if (Ignore(U'{')) {
      Output(Token::push())
    } else if (Ignore(U'}')) {
      Output(Token::pop())
    } else if (Ignore(U'0')) {
      if (Ignore(U'b')) {
        Many(AcceptIf(is_binary) || Ignore(U'_'))
        Output(write_signed(token, 2))
      } else if (Ignore(U'o')) {
        Many(AcceptIf(is_octal) || Ignore(U'_'))
        Output(write_signed(token, 8))
      } else if (Ignore(U'x')) {
        Many(AcceptIf(is_hexadecimal) || Ignore(U'_'))
        Output(write_signed(token, 16))
      } else if (Accept(U'.')) {
        Many(AcceptIf(is_decimal) || Ignore(U'_'))
        Output(write_double(token))
      } else {
        Many(AcceptIf(is_decimal) || Ignore(U'_'))
        Output(write_signed(token, 10))
      }
    } else if (AcceptIf(is_decimal)) {
      Many(AcceptIf(is_decimal) || Ignore(U'_'))
      if (Accept(U'.')) {
        Many(AcceptIf(is_decimal) || Ignore(U'_'))
        Output(write_double(token))
      } else {
        Output(write_signed(token, 10))
      }
    } else if (AcceptIf(is_alphabetic)) {
      Many(AcceptIf(is_alphanumeric))
      throw runtime_error(join("Unknown keyword: \"", token, "\"."));
    } else {
      throw runtime_error(join("Invalid character '", input.front(), "'."));
    }

  }

  bool ignore_comment() {
    rx::ignorer ignore;
    if (Ignore(U'#')) {
      Many(IgnoreIf([](uint32_t rune) { return rune != U'\n'; }))
      Ignore(U'\n');
      return true;
    }
    return false;
  }

# undef Accept
# undef AcceptIf
# undef Ignore
# undef IgnoreIf
# undef Many
# undef Output

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
  rx::output_iterator_t<std::back_insert_iterator<decltype(buffer)>> output;

};

template<class R>
tokenizer_t<R> tokenizer(R&& range) {
  return tokenizer_t<R>(std::move(range));
}

#endif
