#ifndef PROTODATA_STREAM_H
#define PROTODATA_STREAM_H

#include <cstdint>
#include <iosfwd>
#include <vector>

class Stream {
public:
  Stream(std::ostream& stream) : stream(stream) {}
  Stream(const Stream&) = delete;
  Stream(Stream&&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream& operator=(Stream&&) = delete;
  ~Stream();
  template<class I>
  void write(I begin, I end) {
    while (begin != end)
      write(*begin++);
  }
  void write_bit(bool);
  void write(char);
  void write(uint64_t, int);
private:
  void flush();
  std::ostream& stream;
  std::vector<uint8_t> buffer;
};

#endif
