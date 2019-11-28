/*************************************************************************
 *
 * File Name:  socket_ops.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include <flute/Logger.h>
#include <flute/endian.h>
#include <flute/flute-config.h>
#include <flute/socket_ops.h>

#include <cassert>
#include <cstring>

#ifdef FLUTE_HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef FLUTE_HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef FLUTE_HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

namespace flute {

int setSocketCloseOnExec(socket_type fd) {
#if !defined(_WIN32) && defined(FLUTE_HAVE_SETFD)
    auto flags = 0;
    if ((flags = ::fcntl(fd, F_GETFD, nullptr)) < 0) {
        LOG_WARN << "fcntl(" << fd << ", FGETFD)";
        return -1;
    }
    if (!(flags & FD_CLOEXEC)) {
        if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
            LOG_WARN << "fcntl(" << fd << ", FSETFD)";
            return -1;
        }
    }
#endif
    return 0;
}

int setSocketNonblocking(socket_type fd) {
#ifdef _WIN32
    unsigned long nonblocking = 1;
    if (::ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR) {
        LOG_WARN << "fcntl(" << fd << ", F_GETFL)";
        return -1;
    }
#else
    int flags;
    if ((flags = ::fcntl(fd, F_GETFL, NULL)) < 0) {
        LOG_WARN << "fcntl(" << fd << ", F_GETFL)";
        return -1;
    }
    if (!(flags & O_NONBLOCK)) {
        if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
            LOG_WARN << "fcntl(" << fd << ", F_SETFL)";
            return -1;
        }
    }
#endif
    return 0;
}

socket_type socket(int domain, int type, int protocol) {
    return ::socket(domain, type, protocol);
}

socket_type createNonblockingSocket(unsigned short int family) {
    socket_type result = INVALID_SOCKET;
#if defined(SOCK_NONBLOCK) && defined(SOCK_CLOEXEC)
    result = flute::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
#else
    result = flute::socket(family, SOCK_STREAM, IPPROTO_TCP);
    setSocketNonblocking(result);
#endif
    if (result == INVALID_SOCKET) {
        LOG_ERROR << "flute::createNonblockingSocket(" << family << ") failed.";
    }
    return result;
}

int bind(socket_type fd, const sockaddr_storage& addr) {
    socklen_t size = 0;
    if (addr.ss_family == AF_INET) {
        size = sizeof(sockaddr_in);
    } else {
        size = sizeof(sockaddr_in6);
    }
    return ::bind(fd, reinterpret_cast<const sockaddr*>(&addr), size);
}

int connect(socket_type fd, const sockaddr_storage& addr) {
    socklen_t size = 0;
    if (addr.ss_family == AF_INET) {
        size = sizeof(sockaddr_in);
    } else {
        size = sizeof(sockaddr_in6);
    }
    return ::connect(fd, reinterpret_cast<const sockaddr*>(&addr), size);
}

int listen(socket_type fd) {
    return ::listen(fd, SOMAXCONN);
}

socket_type accept(socket_type fd, sockaddr_storage& addr) {
    socket_type connectFd = INVALID_SOCKET;
    socklen_t length = sizeof(addr);
#if defined(FLUTE_HAVE_ACCEPT4) && defined(SOCK_NONBLOCK) && defined(SOCK_CLOEXEC)
    connectFd = ::accept4(fd, reinterpret_cast<sockaddr*>(&addr), &length, SOCK_NONBLOCK | SOCK_CLOEXEC);
#else
    connectFd = ::accept(fd, reinterpret_cast<sockaddr*>(&addr), &length);
    setSocketNonblocking(fd);
    setSocketCloseOnExec(fd);
#endif
    return connectFd;
}

std::int32_t read(socket_type fd, void* buffer, std::size_t size) {
    return ::read(fd, buffer, size);
}

std::int32_t write(socket_type fd, void* buffer, std::size_t size) {
    return ::write(fd, buffer, size);
}

int close(socket_type fd) {
    return ::close(fd);
}

sockaddr_storage getLocalAddr(socket_type fd) {
    sockaddr_storage addr{};
    socklen_t size = static_cast<socklen_t>(sizeof(sockaddr_in6));
    if (::getsockname(fd, (sockaddr*)&addr, &size) < 0) {
        LOG_ERROR << "getsocketname(" << fd << ") failed.";
    }
    return addr;
}

sockaddr_storage getRemoteAddr(socket_type fd) {
    sockaddr_storage addr{};
    socklen_t size = static_cast<socklen_t>(sizeof(sockaddr_in6));
    if (::getpeername(fd, (sockaddr*)&addr, &size) < 0) {
        LOG_ERROR << "getpeername(" << fd << ") failed.";
    }
    return addr;
}

bool isSelfConnect(socket_type fd) {
    auto localAddr = getLocalAddr(fd);
    auto remoteAddr = getRemoteAddr(fd);
    if (localAddr.ss_family == AF_INET) {
        auto local = reinterpret_cast<const sockaddr_in*>(&localAddr);
        auto remote = reinterpret_cast<const sockaddr_in*>(&remoteAddr);
        return local->sin_port == remote->sin_port && local->sin_addr.s_addr == remote->sin_addr.s_addr;
    } else if (localAddr.ss_family == AF_INET6) {
        auto local = reinterpret_cast<const sockaddr_in6*>(&localAddr);
        auto remote = reinterpret_cast<const sockaddr_in6*>(&remoteAddr);
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
        LOG_ERROR << "sockops::fromIpPort";
    }
}

void fromIpPort(const char* ip, std::uint16_t port, sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = host2Network(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
        LOG_ERROR << "sockops::fromIpPort";
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

int getSocketError(socket_type fd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&optval), &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

} // namespace flute