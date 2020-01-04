//
// Created by why on 2020/01/04.
//

#ifndef FLUTE_INTERNAL_H
#define FLUTE_INTERNAL_H

#include <flute/Logger.h>
#include <flute/flute-config.h>
#include <flute/socket_ops.h>

#include <chrono>
#include <cstdint>

#ifdef FLUTE_HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef FLUTE_HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif

namespace flute {

inline std::int64_t currentMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

inline int createInterruptDescriptor(socket_type fds[2]) {
#ifdef FLUTE_HAVE_EVENTFD
    auto fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    fds[0] = fds[1] = fd;
    return 0;
#elif defined(FLUTE_HAVE_PIPE2) && defined(O_NONBLOCK) && defined(O_CLOEXEC)
    return ::pipe2(fds, O_NONBLOCK | O_CLOEXEC);
#elif FLUTE_HAVE_PIPE
    if (::pipe(fds) == 0) {
        if (flute::setSocketCloseOnExec(fds[0]) < 0 || flute::setSocketCloseOnExec(fds[1]) < 0 ||
            flute::setSocketNonblocking(fds[0]) < 0 || flute::setSocketNonblocking(fds[1]) < 0) {
            flute::close(fds[0]);
            flute::close(fds[1]);
            fds[0] = fds[1] = flute::FLUTE_INVALID_SOCKET;
            return -1;
        }
        return 0;
    } else {
        LOG_WARN << "pipe()";
    }
#else
    if (flute::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0) {
        if (flute::setSocketCloseOnExec(fds[0]) < 0 || flute::setSocketCloseOnExec(fds[1]) < 0 ||
            flute::setSocketNonblocking(fds[0]) < 0 || flute::setSocketNonblocking(fds[1]) < 0) {
            flute::closeSocket(fds[0]);
            flute::closeSocket(fds[1]);
            fds[0] = fds[1] = flute::FLUTE_INVALID_SOCKET;
            return -1;
        }
        return 0;
    } else {
        LOG_WARN << "flute::socketpair()";
    }
#endif
    return -1;
}

inline int createEventDescriptor(socket_type fds[2]) {
#if defined(FLUTE_HAVE_PIPE2) && defined(O_NONBLOCK) && defined(O_CLOEXEC)
    return ::pipe2(fds, O_NONBLOCK | O_CLOEXEC);
#elif FLUTE_HAVE_PIPE
    if (::pipe(fds) == 0) {
        if (flute::setSocketCloseOnExec(fds[0]) < 0 || flute::setSocketCloseOnExec(fds[1]) < 0 ||
            flute::setSocketNonblocking(fds[0]) < 0 || flute::setSocketNonblocking(fds[1]) < 0) {
            flute::close(fds[0]);
            flute::close(fds[1]);
            fds[0] = fds[1] = flute::FLUTE_INVALID_SOCKET;
            return -1;
        }
        return 0;
    } else {
        LOG_WARN << "pipe()";
    }
#else
    if (flute::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) == 0) {
        if (flute::setSocketCloseOnExec(fds[0]) < 0 || flute::setSocketCloseOnExec(fds[1]) < 0 ||
            flute::setSocketNonblocking(fds[0]) < 0 || flute::setSocketNonblocking(fds[1]) < 0) {
            flute::closeSocket(fds[0]);
            flute::closeSocket(fds[1]);
            fds[0] = fds[1] = flute::FLUTE_INVALID_SOCKET;
            return -1;
        }
        return 0;
    } else {
        LOG_WARN << "flute::socketpair()";
    }
#endif
    return -1;
}

} // namespace flute

#endif // FLUTE_INTERNAL_H