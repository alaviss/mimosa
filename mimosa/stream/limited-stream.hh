#pragma once

# include "stream.hh"

namespace mimosa
{
  namespace stream
  {
    class LimitedStream : public Stream
    {
    public:
      MIMOSA_DEF_PTR(LimitedStream);

      LimitedStream(Stream::Ptr stream);

      virtual int64_t write(const char * data, uint64_t nbytes);
      virtual int64_t read(char * data, uint64_t nbytes);

      inline void setReadLimit(uint64_t bytes) { rbytes_left_ = bytes; }
      inline void setWriteLimit(uint64_t bytes) { wbytes_left_ = bytes; }

    private:
      Stream::Ptr stream_;
      uint64_t    rbytes_left_;
      uint64_t    wbytes_left_;
    };
  }
}

