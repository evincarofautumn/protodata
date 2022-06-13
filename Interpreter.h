#ifndef PROTODATA_INTERPRETER_H
#define PROTODATA_INTERPRETER_H
#include <Stream.h>
#include <Term.h>

#include <iosfwd>
#include <stack>
#include <vector>

class Interpreter {
public:
  Interpreter(std::ostream&);
  void run(const std::vector<Term>&);
  struct State {
    State();
    Term::Width width;
    Term::Endianness endianness;
    Term::Signedness signedness;
    Term::Format format;
  };

  struct FunctionState {
      FunctionState(Term::Function function);

      Term::Function function;
      std::vector<std::string> parameters;
  };
private:
  Stream output;
  std::stack<State> state;
  std::stack<FunctionState> functionState;
};

#endif
