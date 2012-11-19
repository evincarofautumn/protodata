#ifndef PROTODATA_TOKENIZER_INC
#define PROTODATA_TOKENIZER_INC

#include <iterator>
#include <stdexcept>

#include <rx/ignorer.h>
#include <rx/output_iterator.h>
#include <rx/output_splitter.h>
#include <rx/utf8_output_encoder.h>

#include <chartype.h>
#include <util.h>

template<class R>
tokenizer_t<R> tokenizer(R&& range) {
  return tokenizer_t<R>(std::move(range));
}

template<class R>
tokenizer_t<R>::tokenizer_t(R&& range)
  : input(std::move(range))
  , output(rx::output_iterator(std::back_inserter(buffer))) {
  advance(output);
}

template<class R>
bool tokenizer_t<R>::empty() const {
  return input.empty() && buffer.empty();
}

template<class R>
void tokenizer_t<R>::pop_front() {
  buffer.pop_front();
  advance(output);
}

template<class R>
Token tokenizer_t<R>::front() const {
  return buffer.front();
}

#define Accept(X)   accept(X, input, append)
#define AcceptIf(X) accept_if(X, input, append)
#define Ignore(X)   accept(X, input, ignore)
#define IgnoreIf(X) accept_if(X, input, ignore)
#define Many(X)     while (X)
#define Output(X)   output.put(X)

template<class R>
template<class O>
void tokenizer_t<R>::advance(O output) {

  using namespace rx;
  using namespace std;

  vector<uint32_t> raw;
  string token;
  ignorer ignore;
  auto append = output_splitter(output_iterator(back_inserter(raw)),
    utf8_output_encoder(output_iterator(back_inserter(token))));

  Many(IgnoreIf(is_whitespace) || ignore_comment());
  if (input.empty()) return;

  if (Ignore(U'0')) {
    if (Ignore(U'b')) {
      Many(AcceptIf(is_binary) || Ignore(U'_'));
      Output(write_signed(token, 2));
    } else if (Ignore(U'o')) {
      Many(AcceptIf(is_octal) || Ignore(U'_'));
      Output(write_signed(token, 8));
    } else if (Ignore(U'x')) {
      Many(AcceptIf(is_hexadecimal) || Ignore(U'_'));
      Output(write_signed(token, 16));
    } else if (Accept(U'.')) {
      Many(AcceptIf(is_decimal) || Ignore(U'_'));
      Output(write_double(token));
    } else {
      Many(AcceptIf(is_decimal) || Ignore(U'_'));
      Output(write_signed(token, 10));
    }
  } else if (AcceptIf(is_decimal)) {
    Many(AcceptIf(is_decimal) || Ignore(U'_'));
    if (Accept(U'.')) {
      Many(AcceptIf(is_decimal) || Ignore(U'_'));
      Output(write_double(token));
    } else {
      Output(write_signed(token, 10));
    }
  } else if (Ignore(U'"')) {
    Many(AcceptIf([](uint32_t rune) { return rune != U'"'; }));
    Ignore(U'"');
    for (auto rune : raw)
      Output(Token::write(uint64_t(rune)));
  } else if (AcceptIf(is_alphabetic)) {
    Many(AcceptIf(is_alphanumeric));
    throw runtime_error(join("Unknown keyword: \"", token, "\"."));
  } else if (Ignore(U'{')) {
    Output(Token::push());
  } else if (Ignore(U'}')) {
    Output(Token::pop());
  } else {
    string message("Invalid character: '");
    utf8::append(input.front(), back_inserter(message));
    message += "'.";
    throw runtime_error(message);
  }

}

template<class R>
bool tokenizer_t<R>::ignore_comment() {
  rx::ignorer ignore;
  if (Ignore(U'#')) {
    Many(IgnoreIf([](uint32_t rune) { return rune != U'\n'; }));
    Ignore(U'\n');
    return true;
  }
  return false;
}

#undef Accept
#undef AcceptIf
#undef Ignore
#undef IgnoreIf
#undef Many
#undef Output

#endif
