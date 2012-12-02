#include <deleters.h>

#include <iostream>

void istream_deleter::operator()(std::istream* const pointer) const {
  if (pointer != &std::cin)
    delete pointer;
};

void ostream_deleter::operator()(std::ostream* const pointer) const {
  if (pointer != &std::cout)
    delete pointer;
};
