#ifndef RX_OUTPUT_ITERATOR_H
#define RX_OUTPUT_ITERATOR_H

namespace rx {

template<class I>
class output_iterator_t {
public:
  output_iterator_t(I it_) : it(it_) {}
  template<class T>
  void put(const T& value) { *it++ = value; }
private:
  I it;
};

template<class I>
output_iterator_t<I> output_iterator(I it) {
  return output_iterator_t<I>(it);
}

}

#endif
