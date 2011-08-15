#include "response.hh"

namespace mimosa
{
  namespace http
  {
    Response::Response()
      : status_(200),
        keep_alive_(kStatusOk),
        content_encoding_(kCodingIdentity),
        transfer_encoding_(kCodingIdentity),
        content_length_(0),
        cookies_(),
        unparsed_headers_()
    {
    }

    std::string &&
    Response::toHttpHeader() const
    {
      std::ostringstream os;
      os << "HTTP/1.1 " << status_ << " " << statusToString(status_) <<"\r\n"
         << "Server: mimosa\r\n"
         << "Connection: " << (keep_alive_ ? "Keep-Alive" : "Close") << "\r\n";
      if (content_length_ > 0)
        os << "Content-Length: " << content_length_ << "\r\n";
      for (auto it = cookies_.begin(); it != cookies_.end(); ++it)
      {
        os << "Set-Cookie: " << it->key() << "=" << value->value();
        if (it->isSecure())
          os << "; Secure";
        if (it->isHttpOnly())
          os << "; HttpOnly";
        if (!it->domain().empty())
          os << "; Domain=" << it->domain();
        if (!it->expires().empty())
          os << "; Expires=" << it->expires();
        if (!it->path().empty())
          os << "; Path=" << it->path();
        os << "\r\n";
      }
      os << "\r\n";
    }
  }
}