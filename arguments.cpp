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
      "the cause of failure, if any, is printed on standard error.\n") {}
};

struct missing_value : std::runtime_error {
  missing_value(const std::string& option)
    : runtime_error(join("Missing value for option: '", option, "'.")) {}
};

struct excessive_value : std::runtime_error {
  excessive_value(const std::string& option)
    : runtime_error(join("Too many values for option: '", option, "'.")) {}
};

struct unknown_option : std::runtime_error {
  unknown_option(const std::string& option)
    : runtime_error(join("Unknown option: '", option, "'.")) {}
};

bool streq(const char* const a, const char* const b) {
  return strcmp(a, b) == 0;
}

bool match_argument(const char* const argument,
  const char* const short_name, const char* const long_name) {
  return streq(argument, short_name) || streq(argument, long_name);
}

std::tuple<std::vector<Input>, unique_ostream>
  parse_arguments(int count, const char* const* begin) {
  using namespace std;
  const char* const stdin_name = "STDIN";
  --count;
  ++begin;
  vector<Input> inputs;
  unique_ostream output;
  bool enable_parsing = true;
  const auto end = begin + count;
  for (auto argument = begin; argument != end; ++argument) {
    if (!enable_parsing) {
      inputs.push_back({*argument,
        unique_istream(new ifstream(*argument, ios::binary))});
      continue;
    }
    if (match_argument(*argument, "-h", "--help")) {
      throw print_usage();
    } else if (match_argument(*argument, "-e", "--eval")) {
      if (argument + 1 == end)
        throw missing_value(*argument);
      ++argument;
      inputs.push_back({*argument,
        unique_istream(new istringstream(*argument))});
    } else if (match_argument(*argument, "-o", "--output")) {
      if (output)
        throw excessive_value(*argument);
      if (argument + 1 == end)
        throw missing_value(*argument);
      ++argument;
      output.reset(new ofstream(*argument, ios::binary));
    } else if (streq(*argument, "-")) {
      inputs.push_back({stdin_name, unique_istream(&cin)});
    } else if (streq(*argument, "--")) {
      enable_parsing = false;
    } else if (**argument == '-') {
      throw unknown_option(*argument);
    } else {
      inputs.push_back({*argument,
        unique_istream(new ifstream(*argument, ios::binary))});
    }
  }
  if (inputs.empty())
    inputs.push_back({stdin_name, unique_istream(&cin)});
  if (!output)
    output.reset(&cout);
  return make_tuple(move(inputs), move(output));
}
