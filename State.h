#ifndef PROTODATA_STATE_H
#define PROTODATA_STATE_H

#include <Term.h>

struct State {
  State()
    : width(sizeof(int) == 8 ? Term::WIDTH_64 : Term::WIDTH_32)
    , endianness(Term::NATIVE)
    , signedness(Term::UNSIGNED)
    , format(Term::INTEGER) {}
  Term::Width width;
  Term::Endianness endianness;
  Term::Signedness signedness;
  Term::Format format;
};

#endif
