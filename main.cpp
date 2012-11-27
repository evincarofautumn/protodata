#include <interpret.h>
#include <io.h>
#include <parse.h>
#include <usage.h>

#include <deleters.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

int main(int argc, char** argv) try {
  using namespace std;
  --argc, ++argv;
  if (!(argc >= 0 && argc <= 2)) {
    usage();
    return 1;
  }
  unique_ptr<istream, istream_deleter> input
    (argc < 1 || strcmp(argv[0], "-") == 0
      ? &cin : new ifstream(argv[0]));
  unique_ptr<ostream, ostream_deleter> output
    (argc < 2 || strcmp(argv[1], "-") == 0
      ? &cout : new ofstream(argv[1], ios::binary));
  interpret(parse(read(*input)), *output);
} catch (const std::exception& exception) {
  std::cerr << exception.what() << '\n';
  return 1;
}
