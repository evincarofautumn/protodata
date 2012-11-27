#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stack>
#include <stdexcept>
#include <vector>

#include <vendor/utf8.h>

#include <chartype.h>
#include <deleters.h>
#include <util.h>

#include <Term.h>

void run(std::istream&, std::ostream&);
std::vector<char> read(std::istream&);
std::vector<uint32_t> decode(const std::vector<char>&);
std::vector<Term> parse(const std::vector<uint32_t>&);
void interpret(const std::vector<Term>&, std::ostream&);
void usage();

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
  run(*input, *output);
} catch (const std::exception& exception) {
  std::cerr << exception.what() << '\n';
  return 1;
}

void run(std::istream& input, std::ostream& output) {
  interpret(parse(decode(read(input))), output);
}

std::vector<char> read(std::istream& input) {
  std::vector<char> result;
  std::istreambuf_iterator<char> begin(input), end;
  result.assign(begin, end);
  return result;
}

std::vector<uint32_t> decode(const std::vector<char>& octets) {
  std::vector<uint32_t> result;
  utf8::utf8to32(octets.begin(), octets.end(), std::back_inserter(result));
  return result;
}

Term write_double(const std::string& token) {
  char* boundary;
  const auto value = std::strtod(token.c_str(), &boundary);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error
      (join("Invalid floating-point literal: \"", token, "\""));
  return Term::write(value);
}

Term write_signed(const std::string& token, int base) {
  char* boundary;
  const auto value = std::strtoll(token.c_str(), &boundary, base);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error(join("Invalid base-", base,
      " signed integer literal: \"", token, "\""));
  return Term::write(int64_t(value));
}

Term write_unsigned(const std::string& token, int base) {
  char* boundary;
  const auto value = std::strtoull(token.c_str(), &boundary, base);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error(join("Invalid base-", base,
      " unsigned integer literal: \"", token, "\""));
  return Term::write(uint64_t(value));
}

std::vector<Term> parse(const std::vector<uint32_t>& runes) {
  enum {
    NORMAL,
    COMMENT,
    IDENTIFIER,
    ZERO,
    BINARY,
    OCTAL,
    DECIMAL,
    HEXADECIMAL,
    FLOAT,
    STRING,
    ESCAPE,
  } state = NORMAL;
  std::vector<Term> result;
  std::string token;
  auto append = std::back_inserter(token);
  auto here = runes.begin();
  while (here != runes.end()) {
    switch (state) {
    case NORMAL:
      token.clear();
      if (is_whitespace(*here)) {
        ++here;
      } else if (*here == U'0') {
        utf8::append(*here, append);
        ++here;
        state = ZERO;
      } else if (is_decimal(*here)) {
        utf8::append(*here, append);
        ++here;
        state = DECIMAL;
      } else if (*here == '"') {
        ++here;
        state = STRING;
      } else if (is_alphabetic(*here)) {
        utf8::append(*here, append);
        ++here;
        state = IDENTIFIER;
      } else if (*here == '#') {
        ++here;
        state = COMMENT;
      } else if (*here == '{') {
        ++here;
        result.push_back(Term::push());
      } else if (*here == '}') {
        ++here;
        result.push_back(Term::pop());
      } else {
        std::string message("Invalid character: '");
        utf8::append(*here, std::back_inserter(message));
        message += "'.";
        throw std::runtime_error(message);
      }
      break;
    case COMMENT:
      if (*here == U'\n') {
        ++here;
        state = NORMAL;
      } else {
        ++here;
      }
      break;
    case IDENTIFIER:
      if (is_alphanumeric(*here) || *here == U'_') {
        utf8::append(*here, append);
        ++here;
      } else {
        std::cerr << "Unimplemented command: '" << token << "'.\n";
        state = NORMAL;
      }
      break;
    case ZERO:
      if (*here == U'b') {
        ++here;
        state = BINARY;
      } else if (*here == U'o') {
        ++here;
        state = OCTAL;
      } else if (*here == U'x') {
        ++here;
        state = HEXADECIMAL;
      } else if (*here == U'.') {
        utf8::append(*here, append);
        ++here;
        state = FLOAT;
      } else {
        state = DECIMAL;
      }
      break;
    case BINARY:
      if (is_binary(*here)) {
        utf8::append(*here, append);
        ++here;
      } else if (*here == U'_') {
        ++here;
      } else {
        result.push_back(write_unsigned(token, 2));
        state = NORMAL;
      }
      break;
    case OCTAL:
      if (is_octal(*here)) {
        utf8::append(*here, append);
        ++here;
      } else if (*here == U'_') {
        ++here;
      } else {
        result.push_back(write_unsigned(token, 8));
        state = NORMAL;
      }
      break;
    case DECIMAL:
      if (is_decimal(*here)) {
        utf8::append(*here, append);
        ++here;
      } else if (*here == U'_') {
        ++here;
      } else {
        result.push_back(write_unsigned(token, 10));
        state = NORMAL;
      }
      break;
    case HEXADECIMAL:
      if (is_hexadecimal(*here)) {
        utf8::append(*here, append);
        ++here;
      } else if (*here == U'_') {
        ++here;
      } else {
        result.push_back(write_unsigned(token, 16));
        state = NORMAL;
      }
      break;
    case FLOAT:
      if (is_decimal(*here)) {
        utf8::append(*here, append);
        ++here;
      } else if (*here == U'_') {
        ++here;
      } else {
        result.push_back(write_double(token));
        state = NORMAL;
      }
      break;
    case STRING:
      if (*here == U'"') {
        ++here;
        state = NORMAL;
      } else if (*here == U'\\') {
        ++here;
        state = ESCAPE;
      } else {
        result.push_back(Term::write(Term::Unsigned(*here)));
        ++here;
      }
      break;
    case ESCAPE:
      switch (*here) {
      case U'a':
        result.push_back(Term::write(Term::Unsigned(U'\a')));
        ++here;
        break;
      case U'b':
        result.push_back(Term::write(Term::Unsigned(U'\b')));
        ++here;
        break;
      case U'e':
        result.push_back(Term::write(Term::Unsigned(U'\e')));
        ++here;
        break;
      case U'f':
        result.push_back(Term::write(Term::Unsigned(U'\f')));
        ++here;
        break;
      case U'n':
        result.push_back(Term::write(Term::Unsigned(U'\n')));
        ++here;
        break;
      case U'r':
        result.push_back(Term::write(Term::Unsigned(U'\r')));
        ++here;
        break;
      case U't':
        result.push_back(Term::write(Term::Unsigned(U'\t')));
        ++here;
        break;
      case U'v':
        result.push_back(Term::write(Term::Unsigned(U'\v')));
        ++here;
        break;
      case U'\\':
        result.push_back(Term::write(Term::Unsigned(U'\\')));
        ++here;
        break;
      case U'"':
        result.push_back(Term::write(Term::Unsigned(U'"')));
        ++here;
        break;
      default:
        {
          std::string message("Invalid escape character: '");
          utf8::append(*here, std::back_inserter(message));
          message += "'.";
          throw std::runtime_error(message);
        }
      }
    }
  }
  return result;
}

struct State {
  State()
    : width(sizeof(int) == 8 ? Term::WIDTH_64 : Term::WIDTH_32)
    , endianness(Term::ENDIANNESS_NATIVE)
    , signedness(Term::SIGNEDNESS_UNSIGNED)
    , format(Term::FORMAT_INTEGER) {}
  Term::Width width;
  Term::Endianness endianness;
  Term::Signedness signedness;
  Term::Format format;
};

void interpret(const std::vector<Term>& terms, std::ostream& output) {
  std::vector<char> buffer;
  std::stack<State> state;
  state.push(State());
  for (auto term : terms) {
    switch (term.type) {
    case Term::TYPE_NOOP:
      break;
    case Term::TYPE_PUSH:
      state.push(state.top());
      break;
    case Term::TYPE_POP:
      state.pop();
      break;
    case Term::TYPE_WRITE_SIGNED:
      switch (state.top().format) {
      case Term::FORMAT_INTEGER:
        switch (state.top().signedness) {
        case Term::SIGNEDNESS_UNSIGNED:
          switch (state.top().width) {
          case Term::WIDTH_8:
            {
              const uint8_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          case Term::WIDTH_16:
            {
              const uint16_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          case Term::WIDTH_32:
            {
              const uint32_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          case Term::WIDTH_64:
            {
              const uint64_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          }
          break;
        case Term::SIGNEDNESS_SIGNED:
          switch (state.top().width) {
          case Term::WIDTH_8:
            {
              const int8_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          case Term::WIDTH_16:
            {
              const int16_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          case Term::WIDTH_32:
            {
              const int32_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          case Term::WIDTH_64:
            {
              const int64_t value = term.value.as_signed;
              output.write
                (reinterpret_cast<const char*>(&value), sizeof(value));
            }
            break;
          }
          break;
        }
        break;
      case Term::FORMAT_FLOAT:
        throw std::runtime_error("Unimplemented: floating-point formats.");
        break;
      case Term::FORMAT_UNICODE:
        throw std::runtime_error("Unimplemented: Unicode formats.");
        break;
      }
      {
        const auto begin = reinterpret_cast<const char*>(&term.value.as_signed);
        buffer.assign(begin, begin + sizeof(Term::Signed));
      }
      break;
    case Term::TYPE_WRITE_UNSIGNED:
      throw std::runtime_error("Unimplemented: unsigned values.");
      break;
    case Term::TYPE_WRITE_DOUBLE:
      throw std::runtime_error("Unimplemented: floating-point values.");
      break;
    case Term::TYPE_SET_WIDTH:
      state.top().width = term.value.as_width;
      break;
    case Term::TYPE_SET_FORMAT:
      state.top().format = term.value.as_format;
      break;
    case Term::TYPE_SET_ENDIANNESS:
      state.top().endianness = term.value.as_endianness;
      break;
    }
  }
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
