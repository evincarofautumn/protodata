#include <arguments.h>

#include <util.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

class print_usage : public std::runtime_error {
public:
  print_usage()
    : runtime_error("pd - The Protodata Compiler\n"
      "\n"
      "Usage:\n"
      "\n"
      "\tpd (IN)*\n"
      "\t\t((-e|--eval) STRING)*\n"
      "\t\t((-o|--output) OUT)?\n"
      "\t\t(-- (IN)*)?\n"
      "\n"
      "'pd' takes zero or more Protodata source files (IN), zero or\n"
      "more Protodata one-liners to execute ('-e COMMAND'), and an\n"
      "optional output file ('-o OUT'). It concatenates and\n"
      "compiles all sources into one composite output. If no input\n"
      "or output is specified, then source is read from standard\n"
      "input or compiled to standard output, respectively.\n"
      "\n"
      "To read from standard input explicitly, use a hyphen ('-')\n"
      "in place of a file path. To read from files whose names may\n"
      "begin with dashes, precede them with a double dash ('--').\n"
      "\n"
      "'pd' consumes input and produces output eagerly. It returns\n"
      "0 if all input was consumed, or 1 if there was an error;\n"
      "the cause of failure, if any, is printed on standard error.\n"
) {}
};

class missing_value : public std::runtime_error {
public:
  missing_value(const std::string& option)
    : runtime_error(join("Missing value for option: '", option, "'.")) {}
};

class excessive_value : public std::runtime_error {
public:
  excessive_value(const std::string& option)
    : runtime_error(join("Too many values for option: '", option, "'.")) {}
};

class unknown_option : public std::runtime_error {
public:
  unknown_option(const std::string& option)
    : runtime_error(join("Unknown option: '", option, "'.")) {}
};

std::tuple<std::vector<unique_istream>, unique_ostream>
  parse_arguments(int count, char** begin) {
  using namespace std;
  --count;
  ++begin;
  vector<unique_istream> inputs;
  unique_ostream output;
  bool enable_parsing = true;
  const auto end = begin + count;
  for (auto argument = begin; argument != end; ++argument) {
    if (!enable_parsing) {
      inputs.push_back(unique_istream(new std::ifstream(*argument)));
      continue;
    }
    if (strcmp(*argument, "-h") == 0
      || strcmp(*argument, "--help") == 0) {
      throw print_usage();
    } else if (strcmp(*argument, "-e") == 0
      || strcmp(*argument, "--eval") == 0) {
      if (argument + 1 == end)
        throw missing_value(*argument);
      ++argument;
      inputs.push_back(unique_istream(new std::istringstream(*argument)));
    } else if (strcmp(*argument, "-o") == 0
      || strcmp(*argument, "--output") == 0) {
      if (output)
        throw excessive_value(*argument);
      if (argument + 1 == end)
        throw missing_value(*argument);
      ++argument;
      output.reset(new std::ofstream(*argument, ios::binary));
    } else if (strcmp(*argument, "-") == 0) {
      inputs.push_back(unique_istream(&cin));
    } else if (strcmp(*argument, "--") == 0) {
      enable_parsing = false;
    } else if ((*argument)[0] == '-') {
      throw unknown_option(*argument);
    } else {
      inputs.push_back(unique_istream
        (new std::ifstream(*argument, ios::binary)));
    }
  }
  if (inputs.empty())
    inputs.push_back(unique_istream(&cin));
  if (!output)
    output.reset(&cout);
  return make_tuple(std::move(inputs), std::move(output));
}
