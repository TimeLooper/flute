/*************************************************************************
 *
 * File Name:  socket_ops.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

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

#define BUFFER_MAX_READ_DEFAULT 4096

namespace flute {

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
    DWORD bytesRead;
    DWORD flags = 0;
    WSABUF buf{};
    buf.buf = static_cast<char*>(vec->iov_base);
    buf.len = static_cast<ULONG>(vec->iov_len);
    if (WSARecv(descriptor, &buf, count, &bytesRead, &flags, nullptr, nullptr)) {
        if (WSAGetLastError() == WSAECONNABORTED)
            result = 0;
        else
            result = -1;
    } else {
        result = bytesRead;
    }
    return result;
#endif
}

flute::ssize_t writev(socket_type descriptor, const struct iovec* vec, int count) {
#ifdef FLUTE_HAVE_SYS_UIO_H
    return ::writev(descriptor, vec, count);
#else
    auto result = 0;
    DWORD bytesSend;
    DWORD flags = 0;
    WSABUF buf{};
    buf.buf = static_cast<char*>(vec->iov_base);
    buf.len = static_cast<ULONG>(vec->iov_len);
    if (WSASend(descriptor, &buf, count, &bytesSend, flags, nullptr, nullptr)) {
        if (WSAGetLastError() == WSAECONNABORTED)
            result = 0;
        else
            result = -1;
    } else {
        result = bytesSend;
    }
    return result;
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
#if defined(WIN32) || defined(_WIN32)
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
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(descriptor, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

} // namespace flute