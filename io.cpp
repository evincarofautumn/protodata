#include <io.h>

#include <utf8.h>

#include <iterator>

std::vector<uint32_t> read(std::istream& input) {
  std::vector<char> octets;
  std::vector<uint32_t> result;
  std::istreambuf_iterator<char> begin(input), end;
  octets.assign(begin, end);
  utf8::utf8to32(octets.begin(), octets.end(), std::back_inserter(result));
  return result;
}
