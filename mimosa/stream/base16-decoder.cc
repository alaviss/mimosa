#include <cstring>
#include <memory>
#include <cerrno>

#include "base16-decoder.hh"

namespace mimosa
{
  namespace stream
  {
    Base16Decoder::Base16Decoder(Stream::Ptr stream, const char * base)
      : Filter(stream),
        base_(base)
    {
    }

    uint8_t
    Base16Decoder::decodeByte(uint8_t c) const
    {
      if (base_)
        return ::strchr(base_, c) - base_;

      if ('0' <= c && c <= '9')
        return c - '0';
      if ('a' <= c && c <= 'f')
        return c - 'a' + 10;
      if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
      return 0;
    }

    int64_t
    Base16Decoder::write(const char * data, uint64_t nbytes)
    {
      if (nbytes < 2)
        return 0;

      char buffer[nbytes / 2];

      char * b = buffer;
      const char * const end = data + (nbytes & ~1);
      const char * p = data;

      while (p < end) {
        b[0] = ((decodeByte(p[0]) & 0x0f) << 4) | (decodeByte(p[1]) & 0xf);
        b += 1;
        p += 2;
      }

      if (stream_->loopWrite(buffer, nbytes / 2) == static_cast<int64_t>(nbytes / 2))
        return nbytes;
      return -1;
    }

    int64_t
    Base16Decoder::read(char * /*data*/, uint64_t /*nbytes*/)
    {
      assert(false && "not implemented");
      errno = ENOSYS;
      return -1;
    }
  }
}
