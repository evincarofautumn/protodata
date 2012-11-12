#ifndef RX_IGNORER_H
#define RX_IGNORER_H

namespace rx {

class ignorer {
public:
  ignorer() = default;
  template<class T>
  ignorer(const T&) {}
  bool empty() const { return true; }
  template<class T>
  void put(const T&) {}
};

}

#endif
