#ifndef PROTODATA_INTERPRETER_H
#define PROTODATA_INTERPRETER_H

#include <Term.h>

#include <iosfwd>
#include <stack>
#include <vector>

struct Interpreter {
  Interpreter(std::ostream&);
  void run(const std::vector<Term>&);
  struct State {
    State();
    Term::Width width;
    Term::Endianness endianness;
    Term::Signedness signedness;
    Term::Format format;
  };
private:
  std::ostream& output;
  std::stack<State> state;
};

#endif
