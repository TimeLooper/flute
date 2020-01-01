//
// Created by why on 2019/12/30.
//

#include <flute/Channel.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopInterruptor.h>
#include <flute/Logger.h>
#include <flute/flute-config.h>
#include <flute/socket_ops.h>

#include <functional>

#ifdef FLUTE_HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef FLUTE_HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif

namespace flute {

static int createInterrupterDescriptor(socket_type fds[2]) {
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
    return flute::socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
#endif
    return -1;
}

EventLoopInterruptor::EventLoopInterruptor(EventLoop* loop)
    : m_readDescriptor(FLUTE_INVALID_SOCKET)
    , m_writeDescriptor(FLUTE_INVALID_SOCKET)
    , m_loop(loop)
    , m_channel(nullptr) {
    socket_type fds[2];
    if (createInterrupterDescriptor(fds) != 0) {
        LOG_FATAL << "create event interruptor failed.";
        exit(-1);
    } else {
        m_readDescriptor = fds[0];
        m_writeDescriptor = fds[1];
        m_channel = new Channel(m_readDescriptor, m_loop);
        m_channel->setReadCallback(std::bind(&EventLoopInterruptor::handleRead, this));
        m_channel->enableRead();
    }
}

EventLoopInterruptor::~EventLoopInterruptor() {
    if (m_channel) {
        m_channel->disableAll();
    }
    if (m_readDescriptor != m_writeDescriptor && m_writeDescriptor != FLUTE_INVALID_SOCKET) {
        flute::close(m_writeDescriptor);
        m_writeDescriptor = FLUTE_INVALID_SOCKET;
    }
    if (m_readDescriptor != FLUTE_INVALID_SOCKET) {
        flute::close(m_readDescriptor);
        m_readDescriptor = FLUTE_INVALID_SOCKET;
    }
    delete m_channel;
}

void EventLoopInterruptor::interrupt() const {
    if (m_writeDescriptor == FLUTE_INVALID_SOCKET) {
        return;
    }
    std::uint64_t num = 1;
    flute::write(m_writeDescriptor, &num, sizeof(num));
}

void EventLoopInterruptor::handleRead() {
    std::uint64_t num;
    flute::read(m_readDescriptor, &num, sizeof(num));
}

} // namespace flute