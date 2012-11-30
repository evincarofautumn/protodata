#include <Term.h>

#include <util.h>

Term::Term() : type(NOOP) {}

Term::Term(Endianness endianness) : type(SET_ENDIANNESS) {
  value.as_endianness = endianness;
}

Term::Term(Signedness signedness) : type(SET_SIGNEDNESS) {
  value.as_signedness = signedness;
}

Term::Term(Width width) : type(SET_WIDTH) {
  value.as_width = width;
}

Term::Term(Format format) : type(SET_FORMAT) {
  value.as_format = format;
}

Term::Term(Type type, void* source = 0) : type(type) {
  switch (type) {
  case NOOP:
    break;
  case PUSH:
  case POP:
    break;
  case WRITE_SIGNED:
    value.as_signed = *static_cast<Signed*>(source);
    break;
  case WRITE_UNSIGNED:
    value.as_unsigned = *static_cast<Unsigned*>(source);
    break;
  case WRITE_DOUBLE:
    value.as_double = *static_cast<Double*>(source);
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

Term Term::write(uint64_t value) {
  return Term(WRITE_UNSIGNED, static_cast<void*>(&value));
}

Term Term::write(int64_t value) {
  return Term(WRITE_SIGNED, static_cast<void*>(&value));
}

Term Term::write(double value) {
  return Term(WRITE_DOUBLE, static_cast<void*>(&value));
}
