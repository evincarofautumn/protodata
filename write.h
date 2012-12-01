#ifndef PROTODATA_WRITE_H
#define PROTODATA_WRITE_H

#include <Interpreter.h>

#include <util.h>

#include <utf8.h>

#include <array>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <ostream>
#include <vector>

Term::Endianness platform_endianness();

template<class T>
void endian_copy(const T&, Term::Endianness, std::ostream&);

template<class T>
struct type_name {
  static const char* value;
};

template<class O, class I>
void write_integer_value
  (Term::Endianness endianness,
  const I& input,
  std::ostream& output) {
  typedef std::numeric_limits<decltype(input)> input_limits;
  typedef std::numeric_limits<O> type_limits;
  if (input < type_limits::min() || input > type_limits::max())
    throw std::runtime_error
      (join("Value exceeds range of ", type_name<O>::value, " integer."));
  const O buffer(input);
  endian_copy(buffer, endianness, output);
}

template<class O, class I>
void write_float_value
  (Term::Endianness endianness,
  const I& input,
  std::ostream& output) {
  const O buffer(input);
  endian_copy(buffer, endianness, output);
}

template<class O, class I>
void write_unicode_value
  (Term::Endianness endianness,
  const I& input,
  std::ostream& output,
  O* (*append)(uint32_t, O*)) {
  const uint32_t rune(input);
  std::array<O, sizeof(uint32_t) / sizeof(O)> buffer;
  const auto end(append(rune, &buffer[0]));
  for (auto i = &buffer[0]; i != end; ++i)
    endian_copy(*i, endianness, output);
}

template<class T>
void write_integer
  (const Interpreter::State& state, const T& input, std::ostream& output) {
  const auto& endianness = state.endianness;
  switch (state.format) {
  case Term::INTEGER:
    switch (state.signedness) {
    case Term::UNSIGNED:
      switch (state.width) {
      case Term::WIDTH_8:
        write_integer_value<uint8_t>(endianness, input, output);
        break;
      case Term::WIDTH_16:
        write_integer_value<uint16_t>(endianness, input, output);
        break;
      case Term::WIDTH_32:
        write_integer_value<uint32_t>(endianness, input, output);
        break;
      case Term::WIDTH_64:
        write_integer_value<uint64_t>(endianness, input, output);
        break;
      }
      break;
    case Term::SIGNED:
      switch (state.width) {
      case Term::WIDTH_8:
        write_integer_value<int8_t>(endianness, input, output);
        break;
      case Term::WIDTH_16:
        write_integer_value<int16_t>(endianness, input, output);
        break;
      case Term::WIDTH_32:
        write_integer_value<int32_t>(endianness, input, output);
        break;
      case Term::WIDTH_64:
        write_integer_value<int64_t>(endianness, input, output);
        break;
      }
      break;
    }
    break;
  case Term::FLOAT:
    switch (state.width) {
    case Term::WIDTH_32:
      write_float_value<float>(endianness, input, output);
      break;
    case Term::WIDTH_64:
      write_float_value<double>(endianness, input, output);
      break;
    default:
      IMPOSSIBLE("invalid float bit width");
    }
    break;
  case Term::UNICODE:
    switch (state.width) {
    case Term::WIDTH_8:
      write_unicode_value<uint8_t>(endianness, input, output, utf8::append);
      break;
    case Term::WIDTH_16:
      write_unicode_value<uint16_t>(endianness, input, output, utf8::append16);
      break;
    case Term::WIDTH_32:
      {
        const uint32_t buffer = input;
        endian_copy(buffer, endianness, output);
      }
      break;
    default:
      IMPOSSIBLE("invalid Unicode width");
    }
  }
}

template<class T>
void write_float
  (const Interpreter::State& state, const T& input, std::ostream& output) {
  switch (state.format) {
  case Term::INTEGER:
    throw std::runtime_error
      ("Float values cannot be written in integer format.");
  case Term::FLOAT:
    switch (state.width) {
    case Term::WIDTH_32:
      write_float_value<float>(state.endianness, input, output);
      break;
    case Term::WIDTH_64:
      write_float_value<double>(state.endianness, input, output);
      break;
    default:
      IMPOSSIBLE("invalid float bit width");
    }
    break;
  case Term::UNICODE:
    throw std::runtime_error
      ("Float values cannot be written in Unicode format.");
  }
}

// Conversion via pointer to character type is, to my
// knowledge, the only way of detecting endianness that is
// required to work by the C++ standard.
inline Term::Endianness platform_endianness() {
  const uint16_t value = 0x0001;
  return *reinterpret_cast<const char*>(&value) == 0x01
    ? Term::LITTLE : Term::BIG;
}

template<class T>
void endian_copy
  (const T& value, Term::Endianness endianness, std::ostream& output) {
  using namespace std;
  array<char, sizeof(T)> buffer;
  const auto begin = reinterpret_cast<const char*>(&value),
    end = begin + sizeof(T);
  copy(begin, end, buffer.begin());
  if (endianness != Term::NATIVE && endianness != platform_endianness())
    reverse(buffer.begin(), buffer.end());
  output.write(buffer.begin(), sizeof(T));
}

#endif
