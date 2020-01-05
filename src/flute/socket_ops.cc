//
// Created by why on 2019/12/29.
//

#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/endian.h>
#include <flute/flute-config.h>
#include <flute/socket_ops.h>

#include <cassert>
#include <cstring>

#ifdef FLUTE_HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef FLUTE_HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifdef FLUTE_HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#include <csignal>

#define BUFFER_MAX_READ_DEFAULT 4096

namespace flute {

#ifndef FLUTE_HAVE_SOCKETPAIR

#define FLUTE_SOCKET_ERROR() WSAGetLastError()

#define FLUTE_SET_SOCKET_ERROR(errcode) \
    do {                                \
        WSASetLastError(errcode);       \
    } while (0)

#ifndef AF_UNIX
#define AF_UNIX AF_INET
#endif

int socketpair(int domain, int type, int protocol, socket_type descriptors[2]) {
    /* This code is originally from Tor.  Used with permission. */

    /* This socketpair does not work when localhost is down. So
     * it's really not the same thing at all. But it's close enough
     * for now, and really, when localhost is down sometimes, we
     * have other problems too.
     */
    socket_type listener = -1;
    socket_type connector = -1;
    socket_type acceptor = -1;
    struct sockaddr_in listen_addr;
    struct sockaddr_in connect_addr;
    socklen_t size;
    int saved_errno = -1;
    int family_test;

    family_test = domain != AF_INET;
#ifdef AF_UNIX
    family_test = family_test && (domain != AF_UNIX);
#endif
    if (protocol || family_test) {
        FLUTE_SET_SOCKET_ERROR(FLUTE_ERROR(EAFNOSUPPORT));
        return -1;
    }

    if (!descriptors) {
        FLUTE_SET_SOCKET_ERROR(FLUTE_ERROR(EINVAL));
        return -1;
    }

    listener = ::socket(AF_INET, type, 0);
    if (listener < 0) return -1;
    std::memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listen_addr.sin_port = 0; /* kernel chooses port.	 */
    if (::bind(listener, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) == -1) goto tidy_up_and_fail;
    if (::listen(listener, 1) == -1) goto tidy_up_and_fail;

    connector = ::socket(AF_INET, type, 0);
    if (connector < 0) goto tidy_up_and_fail;

    std::memset(&connect_addr, 0, sizeof(connect_addr));

    /* We want to find out the port number to connect to.  */
    size = sizeof(connect_addr);
    if (::getsockname(listener, (struct sockaddr*)&connect_addr, &size) == -1) goto tidy_up_and_fail;
    if (size != sizeof(connect_addr)) goto abort_tidy_up_and_fail;
    if (::connect(connector, (struct sockaddr*)&connect_addr, sizeof(connect_addr)) == -1) goto tidy_up_and_fail;

    size = sizeof(listen_addr);
    acceptor = ::accept(listener, (struct sockaddr*)&listen_addr, &size);
    if (acceptor < 0) goto tidy_up_and_fail;
    if (size != sizeof(listen_addr)) goto abort_tidy_up_and_fail;
    /* Now check we are talking to ourself by matching port and host on the
       two sockets.	 */
    if (::getsockname(connector, (struct sockaddr*)&connect_addr, &size) == -1) goto tidy_up_and_fail;
    if (size != sizeof(connect_addr) || listen_addr.sin_family != connect_addr.sin_family ||
        listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr || listen_addr.sin_port != connect_addr.sin_port)
        goto abort_tidy_up_and_fail;
    ::closesocket(listener);
    descriptors[0] = connector;
    descriptors[1] = acceptor;

    return 0;

abort_tidy_up_and_fail:
    saved_errno = FLUTE_ERROR(ECONNABORTED);
tidy_up_and_fail:
    if (saved_errno < 0) saved_errno = FLUTE_SOCKET_ERROR();
    if (listener != -1) closesocket(listener);
    if (connector != -1) closesocket(connector);
    if (acceptor != -1) closesocket(acceptor);

    FLUTE_SET_SOCKET_ERROR(saved_errno);
    return -1;
#undef ERR
}
#endif

void initialize() {
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#else
    std::signal(SIGPIPE, SIG_IGN);
#endif
}

void deinitialize() {
#ifdef _WIN32
    WSACleanup();
#endif
}

int setSocketCloseOnExec(socket_type descriptor) {
#if !defined(_WIN32) && defined(FLUTE_HAVE_SETFD)
    auto flags = 0;
    if ((flags = ::fcntl(descriptor, F_GETFD, nullptr)) < 0) {
        LOG_WARN << "fcntl(" << descriptor << ", FGETFD)";
        return -1;
    }
    if (!(flags & FD_CLOEXEC)) {
        if (::fcntl(descriptor, F_SETFD, flags | FD_CLOEXEC) == -1) {
            LOG_WARN << "fcntl(" << descriptor << ", FSETFD)";
            return -1;
        }
    }
#endif
    return 0;
}

int setSocketNonblocking(socket_type descriptor) {
#ifdef _WIN32
    unsigned long nonblocking = 1;
    if (::ioctlsocket(descriptor, FIONBIO, &nonblocking) == SOCKET_ERROR) {
        LOG_WARN << "fcntl(" << descriptor << ", F_GETFL)";
        return -1;
    }
#else
    int flags;
    if ((flags = ::fcntl(descriptor, F_GETFL, NULL)) < 0) {
        LOG_WARN << "fcntl(" << descriptor << ", F_GETFL)";
        return -1;
    }
    if (!(flags & O_NONBLOCK)) {
        if (::fcntl(descriptor, F_SETFL, flags | O_NONBLOCK) == -1) {
            LOG_WARN << "fcntl(" << descriptor << ", F_SETFL)";
            return -1;
        }
    }
#endif
    return 0;
}

socket_type socket(int domain, int type, int protocol) { return ::socket(domain, type, protocol); }

socket_type createNonblockingSocket(unsigned short int family) {
    socket_type result = FLUTE_INVALID_SOCKET;
#if defined(SOCK_NONBLOCK) && defined(SOCK_CLOEXEC)
    result = flute::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
#else
    result = flute::socket(family, SOCK_STREAM, IPPROTO_TCP);
    setSocketNonblocking(result);
#endif
    if (result == FLUTE_INVALID_SOCKET) {
        LOG_ERROR << "flute::createNonblockingSocket(" << family << ") failed.";
    }
    return result;
}

int bind(socket_type descriptor, const InetAddress& addr) {
    socklen_t size = 0;
    if (addr.family() == AF_INET) {
        size = sizeof(sockaddr_in);
    } else {
        size = sizeof(sockaddr_in6);
    }
    return ::bind(descriptor, reinterpret_cast<const sockaddr*>(&addr), size);
}

int connect(socket_type descriptor, const InetAddress& addr) {
    socklen_t size = 0;
    if (addr.family() == AF_INET) {
        size = sizeof(sockaddr_in);
    } else {
        size = sizeof(sockaddr_in6);
    }
    return ::connect(descriptor, addr.getSocketAddress(), size);
}

int listen(socket_type descriptor) { return ::listen(descriptor, SOMAXCONN); }

socket_type accept(socket_type descriptor, InetAddress* addr) {
    socket_type connectFd = FLUTE_INVALID_SOCKET;
    sockaddr_in6 tmp{};
    socklen_t length = sizeof(tmp);
#if defined(FLUTE_HAVE_ACCEPT4) && defined(SOCK_NONBLOCK) && defined(SOCK_CLOEXEC)
    connectFd = ::accept4(descriptor, reinterpret_cast<sockaddr*>(&tmp), &length, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
    connectFd = ::accept(descriptor, reinterpret_cast<sockaddr*>(&tmp), &length);
    setSocketNonblocking(connectFd);
    setSocketCloseOnExec(connectFd);
#endif
    if (addr) {
        *addr = InetAddress(tmp);
    }
    return connectFd;
}

flute::ssize_t readv(socket_type descriptor, const struct iovec* vec, int count) {
#ifdef FLUTE_HAVE_SYS_UIO_H
    return ::readv(descriptor, vec, count);
#else
    auto result = 0;
    for (auto i = 0; i < count; ++i) {
        if (vec[i].iov_len <= 0) {
            break;
        }
        auto temp = ::recv(descriptor, reinterpret_cast<char*>(vec[i].iov_base), static_cast<int>(vec[i].iov_len), 0);
        if (temp > 0) {
            result += temp;
        } else if (temp == 0) {
            return 0;
        } else {
            auto error = FLUTE_SOCKET_ERROR();
            if (error == WSAECONNABORTED) {
                return 0;
            } else {
                FLUTE_SET_SOCKET_ERROR(error);
                return -1;
            }
        }
    }
    return result;
// DWORD bytesRead;
// DWORD flags = 0;
// WSABUF buf{};
// buf.buf = static_cast<char*>(vec->iov_base);
// buf.len = static_cast<ULONG>(vec->iov_len);
// if (WSARecv(descriptor, &buf, count, &bytesRead, &flags, nullptr, nullptr)) {
//     if (WSAGetLastError() == WSAECONNABORTED)
//         result = 0;
//     else
//         result = -1;
// } else {
//     result = bytesRead;
// }
// return result;
#endif
}

flute::ssize_t writev(socket_type descriptor, const struct iovec* vec, int count) {
#ifdef FLUTE_HAVE_SYS_UIO_H
    return ::writev(descriptor, vec, count);
#else
    auto result = 0;
    for (auto i = 0; i < count; ++i) {
        if (vec[i].iov_len <= 0) {
            break;
        }
        auto temp =
            ::send(descriptor, reinterpret_cast<const char*>(vec[i].iov_base), static_cast<int>(vec[i].iov_len), 0);
        if (temp > 0) {
            result += temp;
        } else if (temp == 0) {
            return 0;
        } else {
            auto error = FLUTE_SOCKET_ERROR();
            if (error == WSAECONNABORTED) {
                return 0;
            } else {
                FLUTE_SET_SOCKET_ERROR(error);
                return -1;
            }
        }
    }
    return result;
// DWORD bytesSend;
// DWORD flags = 0;
// WSABUF buf{};
// buf.buf = static_cast<char*>(vec->iov_base);
// buf.len = static_cast<ULONG>(vec->iov_len);
// if (WSASend(descriptor, &buf, count, &bytesSend, flags, nullptr, nullptr)) {
//     if (WSAGetLastError() == WSAECONNABORTED)
//         result = 0;
//     else
//         result = -1;
// } else {
//     result = bytesSend;
// }
// return result;
#endif
}

int close(int descriptor) { return ::close(descriptor); }

flute::ssize_t getByteAvaliableOnSocket(socket_type descriptor) {
#if defined(FIONREAD) && defined(_WIN32)
    u_long lng = BUFFER_MAX_READ_DEFAULT;
    if (ioctlsocket(descriptor, FIONREAD, &lng) < 0) return -1;
    /* Can overflow, but mostly harmlessly. XXXX */
    return lng;
#elif defined(FIONREAD)
    std::int32_t n = BUFFER_MAX_READ_DEFAULT;
    if (ioctl(descriptor, FIONREAD, &n) < 0) return -1;
    return n;
#else
    return BUFFER_MAX_READ_DEFAULT;
#endif
}

int closeSocket(socket_type descriptor) {
#ifdef _WIN32
    return ::closesocket(descriptor);
#else
    return ::close(descriptor);
#endif
}

InetAddress getLocalAddr(socket_type descriptor) {
    sockaddr_in6 addr{};
    socklen_t size = static_cast<socklen_t>(sizeof(sockaddr_in6));
    if (::getsockname(descriptor, reinterpret_cast<sockaddr*>(&addr), &size) < 0) {
        LOG_ERROR << "getdescriptorname(" << descriptor << ") failed.";
    }
    return InetAddress(addr);
}

InetAddress getRemoteAddr(socket_type descriptor) {
    sockaddr_in6 addr{};
    socklen_t size = static_cast<socklen_t>(sizeof(sockaddr_in6));
    if (::getpeername(descriptor, reinterpret_cast<sockaddr*>(&addr), &size) < 0) {
        LOG_ERROR << "getpeername(" << descriptor << ") failed.";
    }
    return InetAddress(addr);
}

bool isSelfConnect(socket_type descriptor) {
    auto localAddr = getLocalAddr(descriptor);
    auto remoteAddr = getRemoteAddr(descriptor);
    if (localAddr.family() == AF_INET) {
        auto local = reinterpret_cast<const sockaddr_in*>(localAddr.getSocketAddress());
        auto remote = reinterpret_cast<const sockaddr_in*>(remoteAddr.getSocketAddress());
        return local->sin_port == remote->sin_port && local->sin_addr.s_addr == remote->sin_addr.s_addr;
    } else if (localAddr.family() == AF_INET6) {
        auto local = reinterpret_cast<const sockaddr_in6*>(localAddr.getSocketAddress());
        auto remote = reinterpret_cast<const sockaddr_in6*>(remoteAddr.getSocketAddress());
        return local->sin6_port == remote->sin6_port &&
               std::memcmp(&local->sin6_addr, &remote->sin6_addr, sizeof(local->sin6_addr)) == 0;
    } else {
        return false;
    }
}

void fromIpPort(const char* ip, std::uint16_t port, sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = host2Network(port);
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        LOG_ERROR << "flute::fromIpPort";
    }
}

void fromIpPort(const char* ip, std::uint16_t port, sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = host2Network(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        LOG_ERROR << "flute::fromIpPort";
    }
}

void toIpPort(const sockaddr* addr, char* dst, std::size_t size) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        ::inet_ntop(AF_INET, &(reinterpret_cast<const sockaddr_in*>(addr))->sin_addr, dst, size);
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        ::inet_ntop(AF_INET6, &(reinterpret_cast<const sockaddr_in6*>(addr))->sin6_addr, dst, size);
    }
    auto length = std::strlen(dst);
    auto port = network2Host(reinterpret_cast<const sockaddr_in*>(addr)->sin_port);
    assert(length < size);
    std::snprintf(dst + length, size - length, ":%u", port);
}

int getSocketError(socket_type descriptor) {
#ifdef _WIN32
    return FLUTE_SOCKET_ERROR();
#else
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(descriptor, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
#endif
}

int getLastError() {
#ifdef _WIN32
    return GetLastError();
#else
    return errno;
#endif
}

std::string formatErrorString(int error) {
#ifdef _WIN32
    LPVOID buf;
    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_MAX_WIDTH_MASK,
                      nullptr, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPTSTR>(&buf), 0,
                      nullptr)) {
        std::string msg = reinterpret_cast<char*>(buf);
        LocalFree(buf);
        return msg;
    } else {
        return "Unknown error.";
    }
#else
    return std::strerror(error);
#endif
}

} // namespace flute