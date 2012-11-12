#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>

#include <rx/algorithm.h>
#include <rx/istreamer.h>
#include <rx/output_iterator.h>
#include <rx/utf8_input_decoder.h>

#include <deleters.h>
#include <tokenizer.h>

void usage();

int main(int argc, char** argv) try {
  using namespace std;
  using namespace rx;
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
  copy(tokenizer(utf8_input_decoder(istreamer(*input))),
    output_iterator(ostream_iterator<Token>(*output, "\n")));
} catch (const std::exception& exception) {
  std::cerr << exception.what() << '\n';
  return 1;
}

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
