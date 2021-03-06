#include <functional>
#include <memory>

#include "log.hh"
#include "server.hh"
#include "server-channel.hh"
#include "../stream/net-fd-stream.hh"
#include "../stream/tls-stream.hh"
#include "../stream/tee-stream.hh"
#include "../stream/fd-stream.hh"

namespace mimosa
{
  namespace http
  {
    Server::Server()
      : read_timeout_(0),
        write_timeout_(0),
        handler_(),
        x509_cred_(nullptr),
        priority_cache_(nullptr),
        dh_params_(nullptr)
    {
    }

    Server::~Server()
    {
      ::gnutls_priority_deinit(priority_cache_);
      ::gnutls_dh_params_deinit(dh_params_);
      if (x509_cred_)
        ::gnutls_certificate_free_credentials(x509_cred_);
    }

    void
    Server::serve(int                fd,
                  const ::sockaddr * address,
                  socklen_t          address_len) const
    {
      stream::Stream::Ptr stream(new stream::NetFdStream(fd, true));
      if (x509_cred_)
      {
        auto tls_stream = new stream::TlsStream(stream, true);
        stream = tls_stream;
        int ret = ::gnutls_priority_set(tls_stream->session(), priority_cache_);
        if (ret != GNUTLS_E_SUCCESS)
          throw nullptr;
        ret = ::gnutls_credentials_set(tls_stream->session(), GNUTLS_CRD_CERTIFICATE, x509_cred_);
        if (ret != GNUTLS_E_SUCCESS)
          throw nullptr;

        ::gnutls_certificate_server_set_request(tls_stream->session(), GNUTLS_CERT_IGNORE);

        do {
          ret = gnutls_handshake(tls_stream->session());
        } while (ret < 0 && !gnutls_error_is_fatal(ret));

        if (ret < 0)
        {
          http_log->error("handshake failed: %s", gnutls_strerror(ret));
          throw nullptr;
        }
      }

      ServerChannel channel(new stream::BufferedStream(stream), handler_);
      channel.setRemoteAddr(address, address_len);
      channel.setReadTimeout(read_timeout_);
      channel.setWriteTimeout(write_timeout_);
      channel.setSsl(x509_cred_);
      channel.run();
    }

    void
    Server::setSecure(const std::string & cert_file,
                      const std::string & key_file)
    {
      int ret;

      if (!priority_cache_) {
        ret = ::gnutls_priority_init(&priority_cache_, "NORMAL", NULL);
        if (ret != GNUTLS_E_SUCCESS)
          throw nullptr;
      }

      if (!dh_params_)
      {
        gnutls_dh_params_init(&dh_params_);
        int bits = gnutls_sec_param_to_pk_bits(GNUTLS_PK_DH, GNUTLS_SEC_PARAM_LOW);
        gnutls_dh_params_generate2(dh_params_, bits);
      }

      if (!x509_cred_)
        gnutls_certificate_allocate_credentials(&x509_cred_);

      gnutls_certificate_set_x509_trust_file(
        x509_cred_, "/etc/ssl/certs/ca-certificates.crt", GNUTLS_X509_FMT_PEM);

      ret = ::gnutls_certificate_set_x509_key_file(
        x509_cred_, cert_file.c_str(), key_file.c_str(), GNUTLS_X509_FMT_PEM);
      if (ret != GNUTLS_E_SUCCESS)
        throw nullptr;

      ::gnutls_certificate_set_dh_params(x509_cred_, dh_params_);
    }
  }
}
