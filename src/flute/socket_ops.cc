//
// Created by why on 2019/12/29.
//

#include <flute/Logger.h>
#include <flute/socket_ops.h>

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

} // namespace flute