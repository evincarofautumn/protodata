#include <parse.h>

#include <chartype.h>
#include <util.h>

#include <utf8.h>

#include <map>
#include <string>

Term write_double_term(const std::string&);
Term write_integer_term(const std::string&, int);

template<class I, class O>
bool accept(uint32_t rune, I& input, O output) {
  if (*input == rune) {
    utf8::append(*input, output);
    ++input;
    return true;
  }
  return false;
}

template<class I>
bool accept(uint32_t rune, I& input) {
  if (*input == rune) {
    ++input;
    return true;
  }
  return false;
}

template<class P, class I, class O>
bool accept_if(P predicate, I& input, O output) {
  if (predicate(*input)) {
    utf8::append(*input++, output);
    return true;
  }
  return false;
}

template<class P, class I>
bool accept_if(P predicate, I& input) {
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
      if (accept_if(is_whitespace, here)
        || transition(state, STRING, U'"', here)
        || transition_if(state, IDENTIFIER, is_alphabetic, here, append)
        || transition(state, COMMENT, U'#', here))
        break;
      if (accept(U'{', here)) {
        result.push_back(Term::push());
      } else if (accept(U'}', here)) {
        result.push_back(Term::pop());
      } else {
        state = NUMBER;
      }
      break;
    case NUMBER:
      if (accept(U'+', here, append)
        || accept(U'-', here, append)
        || transition(state, ZERO, U'0', here, append)
        || transition_if(state, DECIMAL, is_decimal, here, append))
        break;
      {
        std::string message("Invalid character: '");
        utf8::append(*here, std::back_inserter(message));
        message += "'.";
        throw std::runtime_error(message);
      }
    case COMMENT:
      if (transition(state, NORMAL, U'\n', here))
        break;
      ++here;
      break;
    case IDENTIFIER:
      if (accept_if(is_alphanumeric, here, append))
        break;
      {
        const auto command = commands.find(token);
        if (command == commands.end())
          throw std::runtime_error(join
            ("Unimplemented command: '", token, "'.\n"));
        result.insert(result.end(),
          command->second.begin(), command->second.end());
      }
      state = NORMAL;
      break;
    case ZERO:
      if (transition(state, BINARY, U'b', here)
        || transition(state, OCTAL, U'o', here)
        || transition(state, HEXADECIMAL, U'x', here)
        || transition(state, FLOAT, U'.', here, append))
        break;
      state = DECIMAL;
      break;
    case BINARY:
      if (accept_if(is_binary, here, append)
        || (accept(U'_', here)))
        break;
      result.push_back(write_integer_term(token, 2));
      state = NORMAL;
      break;
    case OCTAL:
      if (accept_if(is_octal, here, append)
        || accept(U'_', here))
        break;
      result.push_back(write_integer_term(token, 8));
      state = NORMAL;
      break;
    case DECIMAL:
      if (accept_if(is_decimal, here, append)
        || accept(U'_', here)
        || transition(state, FLOAT, U'.', here, append))
        break;
      result.push_back(write_integer_term(token, 10));
      state = NORMAL;
      break;
    case HEXADECIMAL:
      if (accept_if(is_hexadecimal, here, append)
        || accept(U'_', here))
        break;
      result.push_back(write_integer_term(token, 16));
      state = NORMAL;
      break;
    case FLOAT:
      if (accept_if(is_decimal, here, append)
        || accept(U'_', here))
        break;
      result.push_back(write_double_term(token));
      state = NORMAL;
      break;
    case STRING:
      if (transition(state, NORMAL, U'"', here)
        || transition(state, ESCAPE, U'\\', here))
        break;
      result.push_back(Term::write(Term::Unsigned(*here++)));
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
        result.push_back(Term::write(value));
        ++here;
      }
    }
  }
  return result;
}

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