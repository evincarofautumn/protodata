#include <deleters.h>

#include <iostream>

void istream_deleter::operator()(std::istream* p) const {
  if (p != &std::cin) delete p;
};

void ostream_deleter::operator()(std::ostream* p) const {
  if (p != &std::cout) delete p;
};
