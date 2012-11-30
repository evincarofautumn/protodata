#ifndef PROTODATA_INTERPRET_H
#define PROTODATA_INTERPRET_H

#include <iosfwd>
#include <vector>

#include <Term.h>

void interpret(const std::vector<Term>&, std::ostream&);

#endif
