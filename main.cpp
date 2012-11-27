#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
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

Term write_integer(const std::string& token, int base) {
  const bool has_sign = token[0] == '-' || token[0] == '+';
  char* boundary;
  const auto begin = token.c_str(), end = begin + token.size();
  if (has_sign) {
    Term::Signed value = std::strtoll(begin, &boundary, base);
    if (boundary != end)
      throw std::runtime_error(join("Invalid base-", base,
        " signed integer literal: \"", token, "\""));
    return Term::write(value);
  } else {
    Term::Unsigned value = std::strtoull(begin, &boundary, base);
    if (boundary != end)
      throw std::runtime_error(join("Invalid base-", base,
        " unsigned integer literal: \"", token, "\""));
    return Term::write(value);
  }
}

std::vector<Term> parse(const std::vector<uint32_t>& runes) {
  enum {
    NORMAL,
    NUMBER,
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
  const std::map<std::string, std::vector<Term>> commands {
    { "u8",  { Term::INTEGER, Term::UNSIGNED, Term::WIDTH_8  } },
    { "u16", { Term::INTEGER, Term::UNSIGNED, Term::WIDTH_16 } },
    { "u32", { Term::INTEGER, Term::UNSIGNED, Term::WIDTH_32 } },
    { "u64", { Term::INTEGER, Term::UNSIGNED, Term::WIDTH_64 } },
    { "s8",  { Term::INTEGER, Term::SIGNED,   Term::WIDTH_8  } },
    { "s16", { Term::INTEGER, Term::SIGNED,   Term::WIDTH_16 } },
    { "s32", { Term::INTEGER, Term::SIGNED,   Term::WIDTH_32 } },
    { "s64", { Term::INTEGER, Term::SIGNED,   Term::WIDTH_64 } },
    { "f32", { Term::FLOAT, Term::WIDTH_32 } },
    { "f64", { Term::FLOAT, Term::WIDTH_64 } },
    { "utf8",  { Term::UNICODE, Term::WIDTH_8  } },
    { "utf16", { Term::UNICODE, Term::WIDTH_16 } },
    { "utf32", { Term::UNICODE, Term::WIDTH_32 } },
  };
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
        state = NUMBER;
      }
      break;
    case NUMBER:
      if (*here == U'+' || *here == U'-') {
        utf8::append(*here, append);
      } else if (*here == U'0') {
        utf8::append(*here, append);
        ++here;
        state = ZERO;
      } else if (is_decimal(*here)) {
        utf8::append(*here, append);
        ++here;
        state = DECIMAL;
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
        const auto command = commands.find(token);
        if (command == commands.end())
          throw std::runtime_error(join
            ("Unimplemented command: '", token, "'.\n"));
        result.insert(result.end(),
          command->second.begin(), command->second.end());
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
        result.push_back(write_integer(token, 2));
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
        result.push_back(write_integer(token, 8));
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
        result.push_back(write_integer(token, 10));
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
        result.push_back(write_integer(token, 16));
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
    , endianness(Term::NATIVE)
    , signedness(Term::UNSIGNED)
    , format(Term::INTEGER) {}
  Term::Width width;
  Term::Endianness endianness;
  Term::Signedness signedness;
  Term::Format format;
};

template<class T>
const char* serialize_cast(const T& x) {
  return reinterpret_cast<const char*>(x);
}

namespace utf8 {
  template<class u16_iterator>
  u16_iterator append16(uint32_t rune, u16_iterator result) {
    if (rune > 0xffff) {
      *result++ = static_cast<uint16_t>((rune >> 10) + internal::LEAD_OFFSET);
      *result++ = static_cast<uint16_t>
        ((rune & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
    } else {
      *result++ = static_cast<uint16_t>(rune);
    }
    return result;
  }
}

template<class T>
void write_value(const State& state, const T& input, std::ostream& output) {
  switch (state.format) {
  case Term::INTEGER:
    switch (state.signedness) {
    case Term::UNSIGNED:
      switch (state.width) {
      case Term::WIDTH_8:
        {
          const uint8_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_16:
        {
          const uint16_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_32:
        {
          const uint32_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_64:
        {
          const uint64_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      }
      break;
    case Term::SIGNED:
      switch (state.width) {
      case Term::WIDTH_8:
        {
          const int8_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_16:
        {
          const int16_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_32:
        {
          const int32_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      case Term::WIDTH_64:
        {
          const int64_t buffer = input;
          output.write(serialize_cast(&buffer), sizeof buffer);
        }
        break;
      }
      break;
    }
    break;
  case Term::FLOAT:
    switch (state.width) {
    case Term::WIDTH_32:
      {
        const float buffer = input;
        output.write(serialize_cast(&buffer), sizeof buffer);
      }
      break;
    case Term::WIDTH_64:
      {
        const double buffer = input;
        output.write(serialize_cast(&buffer), sizeof buffer);
      }
      break;
    default:
      IMPOSSIBLE("invalid floating-point width");
    }
    break;
  case Term::UNICODE:
    switch (state.width) {
    case Term::WIDTH_8:
      {
        const uint32_t rune = input;
        std::vector<uint8_t> buffer;
        utf8::append(rune, std::back_inserter(buffer));
        output.write
          (serialize_cast(&buffer[0]), buffer.size() * sizeof buffer.front());
      }
      break;
    case Term::WIDTH_16:
      {
        const uint32_t rune = input;
        std::vector<uint16_t> buffer;
        utf8::append16(rune, std::back_inserter(buffer));
        output.write
          (serialize_cast(&buffer[0]), buffer.size() * sizeof buffer.front());
      }
      break;
    case Term::WIDTH_32:
      {
        const uint32_t buffer = input;
        output.write(serialize_cast(&buffer), sizeof buffer);
      }
      break;
    default:
      IMPOSSIBLE("invalid Unicode width");
    }
  }
}

void interpret(const std::vector<Term>& terms, std::ostream& output) {
  std::stack<State> state;
  state.push(State());
  for (auto term : terms) {
    switch (term.type) {
    case Term::NOOP:
      break;
    case Term::PUSH:
      state.push(state.top());
      break;
    case Term::POP:
      state.pop();
      break;
    case Term::WRITE_SIGNED:
      write_value(state.top(), term.value.as_signed, output);
      break;
    case Term::WRITE_UNSIGNED:
      write_value(state.top(), term.value.as_unsigned, output);
      break;
    case Term::WRITE_DOUBLE:
      throw std::runtime_error("Unimplemented: floating-point value.");
    case Term::SET_ENDIANNESS:
      state.top().endianness = term.value.as_endianness;
      break;
    case Term::SET_SIGNEDNESS:
      state.top().signedness = term.value.as_signedness;
      break;
    case Term::SET_WIDTH:
      state.top().width = term.value.as_width;
      break;
    case Term::SET_FORMAT:
      state.top().format = term.value.as_format;
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
