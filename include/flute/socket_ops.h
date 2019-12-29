//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_SOCKET_OPS_H
#define FLUTE_SOCKET_OPS_H

#include <flute/config.h>
#include <flute/flute_types.h>

#include <cstddef>
#include <cstdint>

#ifdef FLUTE_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef FLUTE_HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef FLUTE_HAVE_NETINET_IN6_H
#include <netinet/in6.h>
#endif
#ifdef FLUTE_HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef FLUTE_HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef FLUTE_HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef FLUTE_HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#ifdef FLUTE_HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef FLUTE_HAVE_UNISTD_H
#include <unistd.h>
#endif

namespace flute {

using ::close;

FLUTE_API_DECL int setSocketCloseOnExec(socket_type descriptor);

FLUTE_API_DECL int setSocketNonblocking(socket_type descriptor);

} // namespace flute

#endif // FLUTE_SOCKET_OPS_H
