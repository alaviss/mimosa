#pragma once

# include <cstdint>
# include <string>

# include <sys/types.h>
# include <sys/socket.h>

# include "../time.hh"
# include "../string-ref.hh"

namespace mimosa
{
  namespace net
  {
    int connect(int socket, const struct sockaddr *address,
                socklen_t address_len, Time timeout = 0);

    int connectToHost(const std::string &host, uint16_t port, Time timeout = 0);

    int connectToUnixSocket(const std::string & path,
                            Time timeout = 0);
  }
}

