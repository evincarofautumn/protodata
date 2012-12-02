#include <Interpreter.h>
#include <arguments.h>
#include <parse.h>

#include <nested_exception.h>
#include <util.h>

#include <utf8.h>

#include <iostream>
#include <stdexcept>

void report(const std::exception&, int = 0);

int main(int argc, char** argv) try {
  using namespace std;
  auto parsed_arguments = parse_arguments(argc, argv);
  const auto inputs = move(get<0>(parsed_arguments));
  const auto output = move(get<1>(parsed_arguments));
  vector<Term> terms;
  Interpreter interpreter(*output);
  for (const auto& input : inputs) try {
    parse(*input.stream, interpreter);
  } catch (...) {
    ::throw_with_nested(runtime_error(join("In input ", input.name, ":")));
  }
} catch (const std::exception& exception) {
  report(exception);
  return 1;
}

void report(const std::exception& exception, int depth) {
  std::cerr << std::string(depth * 2, ' ') << exception.what() << '\n';
  try {
    ::rethrow_if_nested(exception);
  } catch (const std::exception& exception) {
    report(exception, depth + 1);
  } catch (...) {
    return;
  }
}
