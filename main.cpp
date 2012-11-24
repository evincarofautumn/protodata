#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

#include <vendor/utf8.h>

#include <chartype.h>
#include <deleters.h>
#include <util.h>

#include <Token.h>

void run(std::istream&, std::ostream&);
std::vector<char> read(std::istream&);
std::vector<uint32_t> decode(const std::vector<char>&);
std::vector<Token> parse(const std::vector<uint32_t>&);
void interpret(const std::vector<Token>&, std::ostream&);
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

Token write_double(const std::string& token) {
  char* boundary;
  const auto value = std::strtod(token.c_str(), &boundary);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error
      (join("Invalid floating-point literal: \"", token, "\""));
  return Token::write(value);
}

Token write_signed(const std::string& token, int base) {
  char* boundary;
  const auto value = std::strtoll(token.c_str(), &boundary, base);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error(join("Invalid base-", base,
      " signed integer literal: \"", token, "\""));
  return Token::write(int64_t(value));
}

Token write_unsigned(const std::string& token, int base) {
  char* boundary;
  const auto value = std::strtoull(token.c_str(), &boundary, base);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error(join("Invalid base-", base,
      " unsigned integer literal: \"", token, "\""));
  return Token::write(uint64_t(value));
}

std::vector<Token> parse(const std::vector<uint32_t>& runes) {
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
  std::vector<Token> result;
  std::string token;
  auto append = std::back_inserter(token);
  auto here = runes.begin();
  while (here != runes.end()) {
    switch (state) {
    case NORMAL:
      if (is_whitespace(*here)) {
        utf8::append(*here, append);
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
        result.push_back(Token::push());
      } else if (*here == '}') {
        ++here;
        result.push_back(Token::pop());
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
        ++here;
      }
      break;
    case ESCAPE:
      switch (*here) {
      case U'a':
        ++here;
        break;
      case U'b':
        ++here;
        break;
      case U'e':
        ++here;
        break;
      case U'f':
        ++here;
        break;
      case U'n':
        ++here;
        break;
      case U'r':
        ++here;
        break;
      case U't':
        ++here;
        break;
      case U'v':
        ++here;
        break;
      case U'\\':
        ++here;
        break;
      case U'"':
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

void interpret(const std::vector<Token>&, std::ostream& output) {
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
