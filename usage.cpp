#include <usage.h>

#include <iostream>

void usage() {
  std::cout << "Usage:\n"
    "\n"
    "\tpd\n"
    "\tpd IN\n"
    "\tpd IN OUT\n"
    "\n"
    "Where IN is a Protodata source file and OUT is a binary\n"
    "destination file. If not specified, IN defaults to standard\n"
    "input; OUT, to standard output. 'pd' consumes input lazily\n"
    "and produces output eagerly. It returns 0 if all of the\n"
    "input was consumed, or 1 if there was an error; the cause of\n"
    "failure, if any, is printed on standard error.\n";
}
