#include <Term.h>

#include <util.h>

Term::Term() : type(NOOP) {}

Term::Term(const Endianness endianness) : type(SET_ENDIANNESS) {
  value.as_endianness = endianness;
}

Term::Term(const Signedness signedness) : type(SET_SIGNEDNESS) {
  value.as_signedness = signedness;
}

Term::Term(const Width width) : type(SET_WIDTH) {
  value.as_width = width;
}

Term::Term(const Format format) : type(SET_FORMAT) {
  value.as_format = format;
}

Term::Term(const Type type, const void* const source = 0) : type(type) {
  switch (type) {
  case NOOP:
    break;
  case PUSH:
  case POP:
    break;
  case WRITE_SIGNED:
    value.as_signed = *static_cast<const Signed*>(source);
    break;
  case WRITE_UNSIGNED:
    value.as_unsigned = *static_cast<const Unsigned*>(source);
    break;
  case WRITE_DOUBLE:
    value.as_double = *static_cast<const Double*>(source);
    break;
  default:
    IMPOSSIBLE("invalid Term type");
  }
}

Term Term::push() {
  return Term(PUSH);
}

Term Term::pop() {
  return Term(POP);
}

Term Term::write(const uint64_t value) {
  return Term(WRITE_UNSIGNED, static_cast<const void*>(&value));
}

Term Term::write(const int64_t value) {
  return Term(WRITE_SIGNED, static_cast<const void*>(&value));
}

Term Term::write(const double value) {
  return Term(WRITE_DOUBLE, static_cast<const void*>(&value));
}
