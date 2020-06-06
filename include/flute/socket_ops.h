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

class InetAddress;

#ifdef FLUTE_HAVE_SYS_UIO_H
using ::iovec;
using ::msghdr;
#else
struct iovec {
    void* iov_base; /* Pointer to data.  */
    size_t iov_len; /* Length of data.  */
};
/* Structure describing messages sent by
   `sendmsg' and received by `recvmsg'.  */
struct msghdr {
    void* msg_name;        /* Address to send to/receive from.  */
    socklen_t msg_namelen; /* Length of address data.  */

    struct iovec* msg_iov; /* Vector of data to send/receive into.  */
    size_t msg_iovlen;     /* Number of elements in the vector.  */

    void* msg_control;     /* Ancillary data (eg BSD filedesc passing). */
    size_t msg_controllen; /* Ancillary data buffer length.
                  !! The type should be socklen_t but the
                  definition of the kernel is incompatible
                  with this.  */

    DWORD msg_flags; /* Flags on received message.  */
};
#endif

using ::getsockopt;
using ::open;
using ::read;
using ::recv;
using ::send;
using ::setsockopt;
using ::shutdown;
using ::write;

#ifdef FLUTE_HAVE_SOCKETPAIR
using ::socketpair;
#else
FLUTE_API_DECL int socketpair(int domain, int type, int protocol, socket_type descriptors[2]);
#endif

#ifdef _WIN32
#define FLUTE_ERROR(e) WSA##e
#else
#define FLUTE_ERROR(e) e
#endif

enum class SocketType { STREAM_SOCKET, DGRAM_SOCKET };

FLUTE_API_DECL void initialize();

FLUTE_API_DECL void deinitialize();

FLUTE_API_DECL int setSocketCloseOnExec(socket_type descriptor);

FLUTE_API_DECL int setSocketNonblocking(socket_type descriptor);

FLUTE_API_DECL socket_type socket(int domain, int type, int protocol);

FLUTE_API_DECL socket_type createNonblockingSocket(unsigned short int family, SocketType type);

FLUTE_API_DECL int bind(socket_type descriptor, const InetAddress& addr);

FLUTE_API_DECL flute::ssize_t readv(socket_type descriptor, iovec* vec, int count);

FLUTE_API_DECL int connect(socket_type descriptor, const InetAddress& addr);

FLUTE_API_DECL int listen(socket_type descriptor);

FLUTE_API_DECL socket_type accept(socket_type descriptor, InetAddress& addr);

FLUTE_API_DECL flute::ssize_t writev(socket_type descriptor, iovec* vec, int count);

FLUTE_API_DECL int close(int descriptor);

FLUTE_API_DECL flute::ssize_t getByteAvaliableOnSocket(socket_type descriptor);

FLUTE_API_DECL int closeSocket(socket_type descriptor);

FLUTE_API_DECL InetAddress getLocalAddr(socket_type descriptor);

FLUTE_API_DECL InetAddress getRemoteAddr(socket_type descriptor);

FLUTE_API_DECL bool isSelfConnect(socket_type descriptor);

FLUTE_API_DECL void fromIpPort(const char* ip, std::uint16_t port, sockaddr_in* addr);

FLUTE_API_DECL void fromIpPort(const char* ip, std::uint16_t port, sockaddr_in6* addr);

FLUTE_API_DECL void toIpPort(const sockaddr* addr, char* dst, std::size_t size);

FLUTE_API_DECL int getSocketError(socket_type descriptor);

FLUTE_API_DECL int getLastError();

FLUTE_API_DECL std::string formatErrorString(int error);

FLUTE_API_DECL flute::ssize_t sendmsg(socket_type descriptor, const msghdr* message, int flags);

FLUTE_API_DECL flute::ssize_t recvmsg(socket_type descriptor, msghdr* message, int flags);

} // namespace flute

#endif // FLUTE_SOCKET_OPS_H
