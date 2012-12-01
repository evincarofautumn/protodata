#include <Interpreter.h>
#include <arguments.h>
#include <parse.h>

#include <util.h>

#include <utf8.h>

#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) try {
  using namespace std;
  auto parsed_arguments = parse_arguments(argc, argv);
  const auto inputs = move(get<0>(parsed_arguments));
  const auto output = move(get<1>(parsed_arguments));
  vector<Term> terms;
  Interpreter interpreter(*output);
  for (const auto& input : inputs)
    parse(*input, interpreter);
} catch (const utf8::exception& exception) {
  std::cerr << "Failed to decode UTF-8: " << exception.what() << ".\n";
  return 1;
} catch (const std::exception& exception) {
  std::cerr << exception.what() << '\n';
  return 1;
}
