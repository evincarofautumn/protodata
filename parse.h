#ifndef PROTODATA_PARSE_H
#define PROTODATA_PARSE_H

#include <cstdint>
#include <vector>

class Interpreter;

void parse(const std::vector<uint32_t>&, Interpreter&);

#endif
