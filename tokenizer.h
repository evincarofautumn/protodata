#ifndef PROTODATA_TOKENIZER_H
#define PROTODATA_TOKENIZER_H

#include <deque>

#include <rx/output_iterator.h>

#include <Token.h>

template<class R>
class tokenizer_t {
public:

  tokenizer_t(tokenizer_t&&) = default;

  tokenizer_t(R&&);
  bool empty() const;
  void pop_front();
  Token front() const;

  tokenizer_t() = delete;
  tokenizer_t(const tokenizer_t&) = delete;
  tokenizer_t& operator=(tokenizer_t&&) = delete;
  tokenizer_t& operator=(const tokenizer_t&) = delete;

private:

  template<class O>
  void advance(O);
  bool ignore_comment();

  R input;
  std::deque<Token> buffer;
  rx::output_iterator_t<std::back_insert_iterator<decltype(buffer)>> output;

};

Token write_double(const std::string&);
Token write_signed(const std::string&, int);
Token write_unsigned(const std::string&, int);

#include <tokenizer.inc>

#endif
