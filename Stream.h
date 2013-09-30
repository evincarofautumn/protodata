#ifndef PROTODATA_STREAM_H
#define PROTODATA_STREAM_H

#include <cstdint>
#include <iosfwd>
#include <vector>

// A wrapper for writing arbitrary binary data to a standard
// output stream.
class Stream {
public:

  Stream(std::ostream& stream) : stream(stream) {}
  ~Stream();

  // Neither copiable nor movable.
  Stream(const Stream&) = delete;
  Stream(Stream&&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream& operator=(Stream&&) = delete;

  // Writes all values in a range.
  template<class I>
  void write(I begin, I end) {
    while (begin != end)
      write(*begin++);
  }

  // Writes a single bit.
  void write_bit(bool);

  // Writes an octet.
  void write(char);

  // Writes up to 64 arbitrary bits.
  void write(uint64_t, int);

private:
  void flush();
  std::ostream& stream;
  std::vector<uint8_t> buffer;
};

#endif
