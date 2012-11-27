#ifndef PROTODATA_WRITE_H
#define PROTODATA_WRITE_H

#include <State.h>

#include <util.h>

#include <utf8.h>

#include <cstdint>
#include <limits>
#include <ostream>
#include <vector>

// The following macros use 'input' and 'output' non-hygienically.

#define WRITE_INTEGER(TYPE, NAME)                                 \
  do {                                                            \
    typedef std::numeric_limits<decltype(input)> input_limits;    \
    typedef std::numeric_limits<TYPE> type_limits;                \
    if (input < type_limits::min() || input > type_limits::max()) \
      throw std::runtime_error                                    \
        (join("Value exceeds range of ", (NAME), " integer."));   \
    const TYPE buffer = input;                                    \
    output.write(serialize_cast(&buffer), sizeof buffer);         \
  } while (false)

#define WRITE_FLOAT(TYPE)                                 \
  do {                                                    \
    const TYPE buffer = input;                            \
    output.write(serialize_cast(&buffer), sizeof buffer); \
  } while (false)

#define WRITE_UNICODE(TYPE, ACTION)           \
  do {                                        \
    const uint32_t rune = input;              \
    std::vector<TYPE> buffer;                 \
    ACTION(rune, std::back_inserter(buffer)); \
    output.write                              \
      (serialize_cast(&buffer[0]),            \
      buffer.size() * sizeof buffer.front()); \
  } while (false)

template<class T>
void write_integer(const State& state, const T& input, std::ostream& output) {
  switch (state.format) {
  case Term::INTEGER:
    switch (state.signedness) {
    case Term::UNSIGNED:
      switch (state.width) {
      case Term::WIDTH_8:  WRITE_INTEGER(uint8_t,  "unsigned 8-bit");  break;
      case Term::WIDTH_16: WRITE_INTEGER(uint16_t, "unsigned 16-bit"); break;
      case Term::WIDTH_32: WRITE_INTEGER(uint32_t, "unsigned 32-bit"); break;
      case Term::WIDTH_64: WRITE_INTEGER(uint64_t, "unsigned 64-bit"); break;
      }
      break;
    case Term::SIGNED:
      switch (state.width) {
      case Term::WIDTH_8:  WRITE_INTEGER(int8_t,  "signed 8-bit");  break;
      case Term::WIDTH_16: WRITE_INTEGER(int16_t, "signed 16-bit"); break;
      case Term::WIDTH_32: WRITE_INTEGER(int32_t, "signed 32-bit"); break;
      case Term::WIDTH_64: WRITE_INTEGER(int64_t, "signed 64-bit"); break;
      }
      break;
    }
    break;
  case Term::FLOAT:
    switch (state.width) {
    case Term::WIDTH_32: WRITE_FLOAT(float);  break;
    case Term::WIDTH_64: WRITE_FLOAT(double); break;
    default: IMPOSSIBLE("invalid float bit width");
    }
    break;
  case Term::UNICODE:
    switch (state.width) {
    case Term::WIDTH_8:  WRITE_UNICODE(uint8_t,  utf8::append);   break;
    case Term::WIDTH_16: WRITE_UNICODE(uint16_t, utf8::append16); break;
    case Term::WIDTH_32:
      {
        const uint32_t buffer = input;
        output.write(serialize_cast(&buffer), sizeof buffer);
      }
      break;
    default:
      IMPOSSIBLE("invalid Unicode width");
    }
  }
}

template<class T>
void write_float(const State& state, const T& input, std::ostream& output) {
  switch (state.format) {
  case Term::INTEGER:
    throw std::runtime_error
      ("Float values cannot be written in integer format.");
  case Term::FLOAT:
    switch (state.width) {
    case Term::WIDTH_32: WRITE_FLOAT(float); break;
    case Term::WIDTH_64: WRITE_FLOAT(double); break;
    default: IMPOSSIBLE("invalid float bit width");
    }
    break;
  case Term::UNICODE:
    throw std::runtime_error
      ("Float values cannot be written in Unicode format.");
  }
}

#undef WRITE_INTEGER
#undef WRITE_FLOAT
#undef WRITE_UNICODE

#endif
