#ifndef PROTODATA_WRITE_H
#define PROTODATA_WRITE_H

#include <State.h>

#include <util.h>

#include <utf8.h>

#include <cstdint>
#include <ostream>
#include <vector>

template<class T>
void write_integer(const State& state, const T& input, std::ostream& output) {
  switch (state.format) {
  case Term::INTEGER:
    switch (state.signedness) {
    case Term::UNSIGNED:
      switch (state.width) {
      case Term::WIDTH_8:
        {
          const uint8_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_16:
        {
          const uint16_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_32:
        {
          const uint32_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_64:
        {
          const uint64_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      }
      break;
    case Term::SIGNED:
      switch (state.width) {
      case Term::WIDTH_8:
        {
          const int8_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_16:
        {
          const int16_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_32:
        {
          const int32_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_64:
        {
          const int64_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      }
      break;
    }
    break;
  case Term::FLOAT:
    switch (state.width) {
    case Term::WIDTH_32:
      {
        const float buffer = input;
        output.write(serialize_cast(&buffer), sizeof buffer);
      }
      break;
    case Term::WIDTH_64:
      {
        const double buffer = input;
        output.write(serialize_cast(&buffer), sizeof buffer);
      }
      break;
    default:
      IMPOSSIBLE("invalid floating-point width");
    }
    break;
  case Term::UNICODE:
    switch (state.width) {
    case Term::WIDTH_8:
      {
        const uint32_t rune = input;
        std::vector<uint8_t> buffer;
        utf8::append(rune, std::back_inserter(buffer));
        output.write
          (serialize_cast(&buffer[0]), buffer.size() * sizeof buffer.front());
      }
      break;
    case Term::WIDTH_16:
      {
        const uint32_t rune = input;
        std::vector<uint16_t> buffer;
        utf8::append16(rune, std::back_inserter(buffer));
        output.write
          (serialize_cast(&buffer[0]), buffer.size() * sizeof buffer.front());
      }
      break;
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
    case Term::WIDTH_32:
      {
	const float buffer = input;
	output.write(serialize_cast(&buffer), sizeof buffer);
      }
      break;
    case Term::WIDTH_64:
      {
	const double buffer = input;
	output.write(serialize_cast(&buffer), sizeof buffer);
      }
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

#endif
