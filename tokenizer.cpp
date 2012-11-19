#include <tokenizer.h>

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
