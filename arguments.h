#ifndef PROTODATA_ARGUMENTS_H
#define PROTODATA_ARGUMENTS_H

#include <deleters.h>

#include <iosfwd>
#include <memory>
#include <tuple>
#include <vector>

typedef std::unique_ptr<std::istream, istream_deleter> unique_istream;
typedef std::unique_ptr<std::ostream, ostream_deleter> unique_ostream;

struct Input {
  const char* name;
  unique_istream stream;
};

std::tuple<std::vector<Input>, unique_ostream>
  parse_arguments(int, const char* const*);

#endif
