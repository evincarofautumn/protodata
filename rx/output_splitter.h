#ifndef RX_OUTPUT_SPLITTER_H
#define RX_OUTPUT_SPLITTER_H

namespace rx {

template<class R1, class R2>
class output_splitter_t {
public:
  output_splitter_t(R1&& r1, R2&& r2)
    : r1(std::move(r1)), r2(std::move(r2)) {}
  template<class T>
  void put(const T& value) {
    r1.put(value);
    r2.put(value);
  }
private:
  R1 r1;
  R2 r2;
};

template<class R1, class R2>
output_splitter_t<typename std::remove_reference<R1>::type,
  typename std::remove_reference<R2>::type>
output_splitter(R1&& r1, R2&& r2) {
  return output_splitter_t<typename std::remove_reference<R1>::type,
    typename std::remove_reference<R2>::type>(std::move(r1), std::move(r2));
}

}

#endif
