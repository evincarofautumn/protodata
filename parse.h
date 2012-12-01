#ifndef PROTODATA_PARSE_H
#define PROTODATA_PARSE_H

#include <cstdint>
#include <iosfwd>
#include <vector>

class Interpreter;

void parse(std::istream&, Interpreter&);

#endif
