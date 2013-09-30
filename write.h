#ifndef PROTODATA_WRITE_H
#define PROTODATA_WRITE_H

#include <Interpreter.h>

#include <Stream.h>
#include <util.h>

#include <utf8.h>

#include <array>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <ostream>
#include <vector>

// Writes an integer value to a stream according to the
// current interpreter state.
template<class T>
void write_integer(const Interpreter::State&, const T&, Stream&);

// Writes a range-checked integer value directly to a stream
// using 'endian_copy()'.
template<class O, class I>
void write_integer_value(Term::Endianness, const I&, Stream&);

// Writes a Unicode value directly to a stream using the
// given encoding function. Calls 'endian_copy()' for each
// resulting word in the target encoding.
template<class O, class I>
void write_unicode_value(Term::Endianness, const I&, Stream&,
  O* (*)(uint32_t, O*));

// Writes a floating-point value to a stream according to
// the current interpreter state.
template<class T>
void write_float(const Interpreter::State&, const T&, Stream&);

// Writes a floating-point value directly to a stream using
// 'endian_copy()'.
template<class O, class I>
void write_float_value(Term::Endianness, const I&, Stream&);

// Copies a value to a stream, swapping octets according to
// the given and platform endiannesses.
template<class T>
void endian_copy(const T&, Term::Endianness, Stream&);

// Detects the platform endianness dynamically.
Term::Endianness platform_endianness();

// Traits structure giving the display name of a given
// integral type for error messages.
template<class T>
struct type_name;

#include <write_impl.h>

#endif
