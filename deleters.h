#ifndef PROTODATA_DELETERS_H
#define PROTODATA_DELETERS_H

#include <iosfwd>

struct istream_deleter {
  void operator()(std::istream*) const;
};

struct ostream_deleter {
  void operator()(std::ostream* p) const;
};

#endif
