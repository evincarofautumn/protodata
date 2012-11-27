#include <arguments.h>
#include <interpret.h>
#include <io.h>
#include <parse.h>

#include <util.h>

#include <utf8.h>

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) try {
  using namespace std;
  auto parsed_arguments = parse_arguments(argc, argv);
  const auto inputs = std::move(get<0>(parsed_arguments));
  const auto output = std::move(get<1>(parsed_arguments));
  vector<Term> terms;
  for (const auto& input : inputs) {
    const auto parsed = parse(read(*input));
    terms.insert(terms.end(), parsed.begin(), parsed.end());
  }
  interpret(terms, *output);
} catch (const utf8::exception& exception) {
  std::cerr << "Failed to decode UTF-8: " << exception.what() << ".\n";
  return 1;
} catch (const std::exception& exception) {
  std::cerr << exception.what() << '\n';
  return 1;
}
