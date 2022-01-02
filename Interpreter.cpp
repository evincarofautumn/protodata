#include <Interpreter.h>

#include <write.h>
#include <fstream>

Interpreter::Interpreter(std::ostream& output)
  : output(output) {
  state.push(State());
}

Interpreter::State::State() :
  width(sizeof(int) * 8),
  endianness(Term::NATIVE),
  signedness(Term::UNSIGNED),
  format(Term::INTEGER) {}

Interpreter::FunctionState::FunctionState(Term::Function function)
: function(function),
  parameters() {}

void Interpreter::run(const std::vector<Term>& terms) {
  for (auto term : terms) {
    switch (term.type) {
    case Term::NOOP:
      break;
    case Term::PUSH:
      state.push(state.top());
      break;
    case Term::POP:
      if (state.size() <= 1)
        throw std::runtime_error("Mismatched braces.");
      state.pop();
      break;
    case Term::WRITE_SIGNED:
      write_integer(state.top(), term.value.as_signed, output);
      break;
    case Term::WRITE_UNSIGNED:
      if (functionState.empty()) {
        write_integer(state.top(), term.value.as_unsigned, output);
      } else {
        auto& parameters = functionState.top().parameters;

        if (parameters.empty()) {
          parameters.emplace_back("");
        }

        parameters.back() += static_cast<char>(term.value.as_unsigned);
      }
      break;
    case Term::WRITE_DOUBLE:
      write_float(state.top(), term.value.as_double, output);
      break;
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
    case Term::FUNCTION:
      functionState.push(FunctionState(term.value.as_function));
      break;
    case Term::NEXT_PARAMETER:
      functionState.top().parameters.emplace_back("");
      break;
    case Term::FUNCTION_CLOSE_BRACKET: {
      const auto function = functionState.top();
      functionState.pop();
      switch (function.function) {
        case Term::Function::FILE: {
          const auto parameters = function.parameters;

          if (parameters.empty()) {
            throw std::runtime_error(
              "Expected a filename argument to file() function.\n"
            );
          }
          if (parameters.size() > 1) {
            throw std::runtime_error(
              "Only expected a single filename argument to file() function.\n"
            );
          }
          std::ifstream file(parameters[0], std::ios::binary | std::ios::in);
          if (!file) {
            throw std::runtime_error(
              "File '" + parameters[0] + "' not found.\n"
            );
          }

          file.seekg(0, std::ios::end);
          const long fileSize = file.tellg();
          file.seekg(0, std::ios::beg);
          std::vector<char> fileData(fileSize);
          file.read(fileData.data(), fileSize);
          write_binary(fileData, output);
          break;
        }
        default:
          throw std::runtime_error(
            "Unexpected function found: "
            + std::to_string(term.value.as_function)
            + ".\n"
          );
      }
      break;
    }
    default:
      throw std::runtime_error(
        "Unexpected state found: " + std::to_string(term.type) + "\n"
      );
    }
  }
}
