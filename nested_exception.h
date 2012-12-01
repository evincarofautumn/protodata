#ifndef PROTODATA_NESTED_EXCEPTION_H
#define PROTODATA_NESTED_EXCEPTION_H

// Partial implementation of 'std::nested_exception' because
// the version shipped with G++ 4.6 was missing a 'throw()'.
// In C++ we're in the habit of reinventing wheels.

class nested_exception {
  std::exception_ptr pointer;
public:
  nested_exception() throw() : pointer(std::current_exception()) { }
  nested_exception(const nested_exception&) = default;
  nested_exception& operator=(const nested_exception&) = default;
  virtual ~nested_exception() throw() {}
  void rethrow_nested() const {
    std::rethrow_exception(pointer);
  }
};

template<typename E>
struct nested_exception_internal : public E, public nested_exception {
  explicit nested_exception_internal(E&& exception)
    : E(static_cast<E&&>(exception)) {}
  virtual ~nested_exception_internal() throw() {}
};

template<typename E>
inline void throw_with_nested_internal(E&& exception, const nested_exception*) {
  throw exception;
}

template<typename E>
inline void throw_with_nested_internal(E&& exception, ...) {
  throw nested_exception_internal<E>(static_cast<E&&>(exception));
}
  
template<typename E>
struct get_nested_helper {
  static const nested_exception* get(const E& exception) {
    return dynamic_cast<const nested_exception*>(&exception);
  }
};

template<typename E>
struct get_nested_helper<E*> {
  static const nested_exception* get(const E* exception) {
    return dynamic_cast<const nested_exception*>(exception);
  }
};

template<typename E>
inline const nested_exception* get_nested_exception(const E& exception) {
  return get_nested_helper<E>::get(exception);
}

template<typename E>
inline void throw_with_nested(E exception) {
  if (get_nested_exception(exception))
    throw exception;
  throw_with_nested_internal(static_cast<E&&>(exception), &exception);
}

template<typename E>
inline void rethrow_if_nested(const E& exception) {
  if (const nested_exception* nested = get_nested_exception(exception))
    nested->rethrow_nested();
}

inline void rethrow_if_nested(const nested_exception& exception) {
  exception.rethrow_nested();
}

#endif
