#include <cerrno>

#include "reader.hh"

namespace mimosa
{
  namespace archive
  {
    int
    Reader::openCb(struct archive * /*archive*/,
                   void *           /*ctx*/)
    {
      return 0;
    }

    ssize_t
    Reader::readCb(struct archive * /*archive*/,
                   void *           ctx,
                   const void **    buffer)
    {
      Reader *thiz = reinterpret_cast<Reader *>(ctx);
      auto rbytes = thiz->stream_->read(
                  thiz->buffer_->data(), thiz->buffer_->size());
      *buffer = thiz->buffer_->data();
      return rbytes;
    }

    int
    Reader::closeCb(struct archive * /*archive*/,
                    void *            ctx)
    {
      Reader *thiz = reinterpret_cast<Reader *>(ctx);
      thiz->buffer_ = nullptr;
      thiz->stream_ = nullptr;
      return 0;
    }

    int
    Reader::open(stream::Stream::Ptr stream)
    {
      stream_ = stream;
      buffer_ = new stream::Buffer();
      return archive_read_open(
        archive_, this, openCb, readCb, closeCb);
    }

    int
    Reader::nextHeader(Entry &entry)
    {
      return archive_read_next_header2(archive_, entry);
    }

    int64_t
    Reader::read(char *data, uint64_t nbytes)
    {
      return archive_read_data(archive_, data, nbytes);
    }

    int64_t
    Reader::write(const char *, uint64_t)
    {
      assert(false && "should not be called");
      errno = EINVAL;
      return -1;
    }
  }
}