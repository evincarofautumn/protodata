#ifndef PROTODATA_DELETERS_H
#define PROTODATA_DELETERS_H

struct istream_deleter { void operator()(std::istream* p) const {
  if (p != &std::cin) delete p;
}};

struct ostream_deleter { void operator()(std::ostream* p) const {
  if (p != &std::cout) delete p;
}};

#endif
