#include <interpret.h>

#include <State.h>
#include <write.h>

#include <stack>

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
      write_integer(state.top(), term.value.as_signed, output);
      break;
    case Term::WRITE_UNSIGNED:
      write_integer(state.top(), term.value.as_unsigned, output);
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
    }
  }
}
