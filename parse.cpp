#include <parse.h>

#include <Interpreter.h>
#include <Term.h>
#include <chartype.h>
#include <nested_exception.h>
#include <util.h>

#include <utf8.h>

#include <limits>
#include <map>
#include <string>

namespace {

typedef std::numeric_limits<double> double_limits;

typedef std::vector<Term> Terms;
typedef std::pair<std::string, Terms> Command;
const std::map<std::string, Terms> commands {
  Command("f32",     Terms { Term::FLOAT, Term::Width(32) }),
  Command("f64",     Terms { Term::FLOAT, Term::Width(64) }),
  Command("utf8",    Terms { Term::UNICODE, Term::Width(8)  }),
  Command("utf16",   Terms { Term::UNICODE, Term::Width(16) }),
  Command("utf32",   Terms { Term::INTEGER, Term::Width(32) }),
  Command("ucs2",    Terms { Term::INTEGER, Term::Width(16) }),
  Command("native",  Terms { Term::NATIVE }),
  Command("little",  Terms { Term::LITTLE }),
  Command("big",     Terms { Term::BIG }),
  Command("epsilon", Terms { Term::write(double_limits::epsilon()) }),
  Command("nan",     Terms { Term::write(double_limits::quiet_NaN()) }),
  Command("+inf",    Terms { Term::write(double_limits::infinity()) }),
  Command("-inf",    Terms { Term::write(-double_limits::infinity()) }),
};

template<class I, class O>
bool accept(uint32_t, I&, I, O);

template<class I>
bool accept(uint32_t, I&, I);

template<class P, class I, class O>
bool accept_if(P, I&, I, O);

template<class P, class I>
bool accept_if(P, I&, I);

template<class T, class... Args>
bool transition(T&, const T&, Args&&...);

template<class T, class... Args>
bool transition_if(T&, const T&, Args&&...);

enum State {
  NORMAL,
  NUMBER,
  COMMENT,
  IDENTIFIER,
  SIGNED,
  UNSIGNED,
  ZERO,
  BINARY,
  OCTAL,
  DECIMAL,
  HEXADECIMAL,
  FLOAT,
  STRING,
  ESCAPE,
};

Term write_double_term(const std::string&);
Term write_integer_term(const std::string&, int);
unsigned long parse_width(const std::string&);

}

void parse_internal
  (std::istream&, Interpreter&, unsigned int&, unsigned int&);

void parse(std::istream& input, Interpreter& interpreter) {
  unsigned int line = 1;
  unsigned int column = 0;
  try {
    parse_internal(input, interpreter, line, column);
  } catch (...) {
    ::throw_with_nested(std::runtime_error
      (join("At line ", line, ", column ", column, ":")));
  }
}

void parse_internal(std::istream& input, Interpreter& interpreter,
  unsigned int& line, unsigned int& column) {
  State state = NORMAL;
  std::vector<Term> terms;
  std::string token;
  auto append = std::back_inserter(token);
  std::vector<uint32_t> runes;
  auto here = runes.begin(), end = runes.end();
  while (true) {
    if (here == end) {
      std::string buffer;
      std::getline(input, buffer);
      runes.clear();
      utf8::utf8to32(buffer.begin(), buffer.end(), std::back_inserter(runes));
      if (!input.eof()) {
        runes.push_back(U'\n');
        ++line;
      }
      here = runes.begin();
      end = runes.end();
    }
    switch (state) {
    case NORMAL:
      token.clear();
      column = here - runes.begin();
      if (accept_if(is_whitespace, here, end)
        || transition(state, STRING, U'"', here, end)
        || transition(state, UNSIGNED, U'u', here, end, append)
        || transition(state, SIGNED, U's', here, end, append)
        || transition_if(state, IDENTIFIER, is_alphabetic, here, end, append)
        || transition(state, COMMENT, U'#', here, end)
        || transition(state, NUMBER, U'+', here, end, append)
        || transition(state, NUMBER, U'-', here, end, append))
        break;
      if (accept(U'{', here, end))
        terms.push_back(Term::push());
      else if (accept(U'}', here, end))
        terms.push_back(Term::pop());
      else
        state = NUMBER;
      break;
    case NUMBER:
      if (transition(state, ZERO, U'0', here, end, append)
        || transition_if(state, DECIMAL, is_decimal, here, end, append)
        || transition_if(state, IDENTIFIER, is_alphabetic, here, end, append))
        break;
      if (here == end)
        return;
      {
        std::string message("Invalid character: '");
        utf8::append(*here, std::back_inserter(message));
        message += "'.";
        throw std::runtime_error(message);
      }
    case COMMENT:
      if (transition(state, NORMAL, U'\n', here, end))
        break;
      if (here == end)
        return;
      ++here;
      break;
    case IDENTIFIER:
      if (accept_if(is_alphanumeric, here, end, append))
        break;
      {
        const auto command = commands.find(token);
        if (command == commands.end())
          throw std::runtime_error(join
            ("Unimplemented command: '", token, "'.\n"));
        terms.insert(terms.end(),
          command->second.begin(), command->second.end());
      }
      state = NORMAL;
      break;
    case UNSIGNED:
      if (accept_if(is_decimal, here, end, append)
        || transition_if(state, IDENTIFIER, is_alphabetic, here, end, append)
        || transition(state, IDENTIFIER, U'_', here, end, append))
        break;
      {
        const auto width = parse_width(token);
        terms.push_back(Term::INTEGER);
        terms.push_back(Term::UNSIGNED);
        terms.push_back(Term::Width(width));
        state = NORMAL;
      }
      break;
    case SIGNED:
      if (accept_if(is_decimal, here, end, append)
        || transition_if(state, IDENTIFIER, is_alphabetic, here, end, append)
        || transition(state, IDENTIFIER, U'_', here, end, append))
        break;
      {
        const auto width = parse_width(token);
        terms.push_back(Term::INTEGER);
        terms.push_back(Term::SIGNED);
        terms.push_back(Term::Width(width));
        state = NORMAL;
      }
      break;
    case ZERO:
      if (transition(state, BINARY, U'b', here, end)
        || transition(state, OCTAL, U'o', here, end)
        || transition(state, HEXADECIMAL, U'x', here, end)
        || transition(state, FLOAT, U'.', here, end, append))
        break;
      state = DECIMAL;
      break;
    case BINARY:
      if (accept_if(is_binary, here, end, append)
        || (accept(U'_', here, end)))
        break;
      terms.push_back(write_integer_term(token, 2));
      state = NORMAL;
      break;
    case OCTAL:
      if (accept_if(is_octal, here, end, append)
        || accept(U'_', here, end))
        break;
      terms.push_back(write_integer_term(token, 8));
      state = NORMAL;
      break;
    case DECIMAL:
      if (accept_if(is_decimal, here, end, append)
        || accept(U'_', here, end)
        || transition(state, FLOAT, U'.', here, end, append))
        break;
      terms.push_back(write_integer_term(token, 10));
      state = NORMAL;
      break;
    case HEXADECIMAL:
      if (accept_if(is_hexadecimal, here, end, append)
        || accept(U'_', here, end))
        break;
      terms.push_back(write_integer_term(token, 16));
      state = NORMAL;
      break;
    case FLOAT:
      if (accept_if(is_decimal, here, end, append)
        || accept(U'_', here, end))
        break;
      terms.push_back(write_double_term(token));
      state = NORMAL;
      break;
    case STRING:
      if (transition(state, NORMAL, U'"', here, end)
        || transition(state, ESCAPE, U'\\', here, end))
        break;
      if (here == end)
        throw std::runtime_error("Unexpected end of file in string.");
      terms.push_back(Term::write(Term::Unsigned(*here++)));
      break;
    case ESCAPE:
      {
        Term::Unsigned value = 0;
        switch (*here) {
        case U'"': value = U'"'; break;
        case U'a': value = U'\a'; break;
        case U'b': value = U'\b'; break;
        case U'e': value = U'\e'; break;
        case U'f': value = U'\f'; break;
        case U'n': value = U'\n'; break;
        case U'r': value = U'\r'; break;
        case U't': value = U'\t'; break;
        case U'v': value = U'\v'; break;
        case U'\\': value = U'\\'; break;
        default:
          {
            std::string message("Invalid escape character: '");
            utf8::append(*here, std::back_inserter(message));
            message += "'.";
            throw std::runtime_error(message);
          }
        }
        terms.push_back(Term::write(value));
        ++here;
      }
    }
    interpreter.run(terms);
    terms.clear();
  }
}

namespace {

Term write_double_term(const std::string& token) {
  char* boundary;
  const auto value = std::strtod(token.c_str(), &boundary);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error
      (join("Invalid floating-point literal: \"", token, "\""));
  return Term::write(value);
}

Term write_integer_term(const std::string& token, int base) {
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

unsigned long parse_width(const std::string& token) {
  const auto begin = token.c_str() + 1;
  char* boundary;
  const auto width = std::strtoul(begin, &boundary, 10);
  if (boundary != token.c_str() + token.size())
    throw std::runtime_error
      (join("Invalid unsigned type: '", token, "'."));
  if (width == 0 || width > 64)
    throw std::runtime_error
      (join("Unsupported width of unsigned type: '", token, "'."));
  return width;
}

template<class I, class O>
bool accept(uint32_t rune, I& input, I end, O output) {
  if (input == end)
    return false;
  if (*input == rune) {
    utf8::append(*input, output);
    ++input;
    return true;
  }
  return false;
}

template<class I>
bool accept(uint32_t rune, I& input, I end) {
  if (input == end)
    return false;
  if (*input == rune) {
    ++input;
    return true;
  }
  return false;
}

template<class P, class I, class O>
bool accept_if(P predicate, I& input, I end, O output) {
  if (input == end)
    return false;
  if (predicate(*input)) {
    utf8::append(*input++, output);
    return true;
  }
  return false;
}

template<class P, class I>
bool accept_if(P predicate, I& input, I end) {
  if (input == end)
    return false;
  if (predicate(*input)) {
    ++input;
    return true;
  }
  return false;
}

template<class T, class... Args>
bool transition(T& state, const T& target, Args&&... args) {
  if (accept(std::forward<Args>(args)...)) {
    state = target;
    return true;
  }
  return false;
}

template<class T, class... Args>
bool transition_if(T& state, const T& target, Args&&... args) {
  if (accept_if(std::forward<Args>(args)...)) {
    state = target;
    return true;
  }
  return false;
}

}
