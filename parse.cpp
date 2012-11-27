#include <parse.h>

#include <chartype.h>
#include <util.h>

#include <utf8.h>

#include <map>
#include <string>

Term write_double_term(const std::string&);
Term write_integer_term(const std::string&, int);

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
        result.push_back(write_integer_term(token, 2));
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
        result.push_back(write_integer_term(token, 8));
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
        result.push_back(write_integer_term(token, 10));
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
        result.push_back(write_integer_term(token, 16));
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
        result.push_back(write_double_term(token));
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
