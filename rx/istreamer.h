#ifndef ISTREAMER_H
#define ISTREAMER_H
#include <iosfwd>

namespace rx {

class istreamer {
public:
  istreamer(std::istream& stream) : stream(&stream) { advance(); }
  istreamer(istreamer&& other)
    : stream(other.stream)
    , current(other.current) {
    other.stream = 0;
  }
  bool empty() const { return !stream || !*stream; }
  void pop_front() { advance(); }
  char front() const { return current; }
private:
  void advance() { stream->get(current); }
  std::istream* stream;
  char current;
};

}

#endif
