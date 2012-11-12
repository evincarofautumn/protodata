#ifndef RX_CONTAINER_H
#define RX_CONTAINER_H

#include <rx/forward_iterator.h>

namespace rx {

template<class T>
auto container(const T& value) -> forward_iterator_t<decltype(value.begin())> {
  return forward_iterator(value.begin(), value.end());
}

}

#endif
