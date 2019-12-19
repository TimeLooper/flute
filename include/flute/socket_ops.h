/*************************************************************************
 *
 * File Name:  socket_ops.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#pragma once

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

class InetAddress;

#ifdef FLUTE_HAVE_SYS_UIO_H
using ::iovec;
#else
struct iovec {
    void* iov_base;      /* Pointer to data.  */
    std::size_t iov_len; /* Length of data.  */
};
#endif

using ::getsockopt;
using ::open;
using ::setsockopt;
using ::shutdown;
using ::send;
using ::recv;
using ::read;
using ::write;

FLUTE_API_DECL int setSocketCloseOnExec(socket_type fd);

FLUTE_API_DECL int setSocketNonblocking(socket_type fd);

FLUTE_API_DECL socket_type socket(int domain, int type, int protocol);

FLUTE_API_DECL socket_type createNonblockingSocket(unsigned short int family);

FLUTE_API_DECL int bind(socket_type fd, const InetAddress& addr);

FLUTE_API_DECL flute::ssize_t readv(socket_type fd, const struct iovec* vec, int count);

FLUTE_API_DECL int connect(socket_type fd, const InetAddress& addr);

FLUTE_API_DECL int listen(socket_type fd);

FLUTE_API_DECL socket_type accept(socket_type fd, InetAddress& addr);

FLUTE_API_DECL flute::ssize_t writev(socket_type fd, const struct iovec* vec, int count);

FLUTE_API_DECL int close(int fd);

FLUTE_API_DECL flute::ssize_t getByteAvaliableOnSocket(socket_type descriptor);

FLUTE_API_DECL int closeSocket(socket_type socket);

FLUTE_API_DECL InetAddress getLocalAddr(socket_type fd);

FLUTE_API_DECL InetAddress getRemoteAddr(socket_type fd);

FLUTE_API_DECL bool isSelfConnect(socket_type fd);

FLUTE_API_DECL void fromIpPort(const char* ip, std::uint16_t port, sockaddr_in* addr);

FLUTE_API_DECL void fromIpPort(const char* ip, std::uint16_t port, sockaddr_in6* addr);

FLUTE_API_DECL void toIpPort(const sockaddr* addr, char* dst, std::size_t size);

FLUTE_API_DECL int getSocketError(socket_type fd);

} // namespace flute