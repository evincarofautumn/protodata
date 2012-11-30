#ifndef PROTODATA_IO_H
#define PROTODATA_IO_H

#include <cstdint>
#include <iosfwd>
#include <vector>

std::vector<uint32_t> read(std::istream&);

#endif
