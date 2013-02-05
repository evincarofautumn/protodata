#include <Stream.h>

#include <ostream>

// Extra bits are padded with trailing zero bits to fill the
// last octet of the stream.
Stream::~Stream() {
  const auto extra = buffer.size() % 8;
  if (extra != 0)
    for (int i = 0; i < (8 - extra); ++i)
      buffer.push_back(false);
  flush();
}

// Writes an octet. If the write is aligned on an octet
// boundary, it is also unbuffered.
void Stream::write(const char raw) {
  const uint8_t octet = raw;
  if (buffer.empty()) {
    stream.put(octet);
  } else {
    for (int i = 7; i >= 0; --i)
      buffer.push_back(bool(octet & (1 << i)));
  }
  flush();
}

void Stream::write(const uint64_t data, const int bits) {
  for (int i = bits - 1; i >= 0; --i)
    buffer.push_back(bool(data & (1 << i)));
  flush();
}

void Stream::write_bit(const bool bit) {
  buffer.push_back(bit);
  flush();
}

// Flushes buffered bits, one octet at a time.
void Stream::flush() {
  while (buffer.size() >= 8) {
    uint8_t octet = 0;
    for (int i = 0; i < 8; ++i)
      octet |= *(buffer.begin() + i) << (7 - i);
    buffer.erase(buffer.begin(), buffer.begin() + 8);
    stream.put(octet);
  }
}
