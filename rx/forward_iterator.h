#ifndef RX_FORWARD_ITERATOR_H
#define RX_FORWARD_ITERATOR_H

namespace rx {

template<class I>
class forward_iterator_t {
public:
  forward_iterator_t(I first, I last) : begin(first), end(last) {}
  bool empty() const { return begin == end; }
  typename I::value_type front() const { return *begin; }
  void pop_front() { ++begin; }
private:
  I begin, end;
};

template<class I>
forward_iterator_t<I> forward_iterator(I first, I last) {
  return forward_iterator_t<I>(first, last);
}

}

#endif
