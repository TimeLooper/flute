//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_TYPES_H
#define FLUTE_TYPES_H

#include <flute/flute-config.h>

#include <functional>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <io.h>
#include <process.h>
#include <tchar.h>
#undef _WIN32_WINNT
/* For structs needed by GetAdaptersAddresses */
#define _WIN32_WINNT 0x0501
#include <iphlpapi.h>
#endif
#ifdef FLUTE_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

namespace flute {

#ifdef _WIN32
typedef SOCKET socket_type;
#define SHUT_WR SD_SEND
#define SHUT_RD SD_RECEIVE
#else
typedef int socket_type;
#endif

const socket_type FLUTE_INVALID_SOCKET = static_cast<socket_type>(~0);

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

enum SocketOpCode {
    None,
    Read,
    Write,
    Connect,
    Accept
};

#ifdef WIN32
#if FLUTE_SIZEOF_PTR == 4
typedef int ssize_t;
#elif FLUTE_SIZEOF_PTR == 8
typedef long int ssize_t;
#else
#error
#endif
#else
typedef ::ssize_t ssize_t;
#endif

class TcpConnection;
class UdpConnection;
class RingBuffer;
class InetAddress;
class UdpServer;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void(const TcpConnectionPtr&, RingBuffer&)> MessageCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, flute::ssize_t)> HighWaterMarkCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionEstablishedCallback;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionDestroyCallback;
typedef std::function<void(const socket_type)> AcceptCallback;
typedef std::function<void(const socket_type)> ConnectCallback;

typedef std::function<void(const std::shared_ptr<UdpServer>&, const InetAddress& address, RingBuffer& buffer)>
    UdpMessageCallback;

} // namespace flute

#endif // FLUTE_TYPES_H
