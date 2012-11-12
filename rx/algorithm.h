#ifndef RX_ALGORITHM_H
#define RX_ALGORITHM_H

namespace rx {

template<class I, class O>
void copy(I&& in, O&& out) {
  while (!in.empty()) {
    out.put(in.front());
    in.pop_front();
  }
}

template<class T, class I, class O>
bool accept(const T& expected, I&& input, O&& output) {
  if (input.empty()) return false;
  const auto value(input.front());
  const bool match = expected == value;
  if (match) {
    input.pop_front();
    output.put(value);
  }
  return match;
}

template<class P, class I, class O>
bool accept_if(P predicate, I&& input, O&& output) {
  if (input.empty()) return false;
  const auto value(input.front());
  const bool match = predicate(value);
  if (match) {
    input.pop_front();
    output.put(value);
  }
  return match;
}

}

#endif
