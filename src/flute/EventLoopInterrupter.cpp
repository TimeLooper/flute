/*************************************************************************
 *
 * File Name:  EventLoopInterrupter.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29
 *
 *************************************************************************/

#include <flute/Channel.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopInterrupter.h>
#include <flute/Logger.h>
#include <flute/flute-config.h>
#include <flute/socket_ops.h>

#ifdef FLUTE_HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef FLUTE_HAVE_SYS_EVENTFD_H
#include <sys/eventfd.h>
#endif

namespace flute {

static int createInterrupterDescriptor(socket_type fds[2]) {
#ifdef FLUTE_HAVE_EVENTFD
    auto fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    fds[0] = fds[1] = fd;
    return 0;
#elif defined(FLUTE_HAVE_PIPE2) && defined(O_NONBLOCK) && defined(O_CLOEXEC)
    return pipe2(fds, O_NONBLOCK | O_CLOEXEC);
#elif FLUTE_HAVE_PIPE
    if (pipe(fds) == 0) {
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
#endif
    return -1;
}

EventLoopInterrupter::EventLoopInterrupter(EventLoop* loop)
    : m_read_descriptor(FLUTE_INVALID_SOCKET)
    , m_write_descriptor(FLUTE_INVALID_SOCKET)
    , m_channel(nullptr)
    , m_loop(loop) {
    open();
}

EventLoopInterrupter::~EventLoopInterrupter() { close(); }

void EventLoopInterrupter::interrupt() {
    if (m_write_descriptor != FLUTE_INVALID_SOCKET) {
        std::uint64_t num = 1;
        flute::write(m_write_descriptor, &num, sizeof(num));
    }
}

void EventLoopInterrupter::open() {
    socket_type fds[2];
    auto ret = createInterrupterDescriptor(fds);
    if (ret == 0) {
        m_read_descriptor = fds[0];
        m_write_descriptor = fds[1];
        m_channel = new Channel(m_read_descriptor, m_loop);
        m_channel->enableRead();
        m_channel->setReadCallback(std::bind(&EventLoopInterrupter::handleRead, this));
    }
}

void EventLoopInterrupter::close() {
    if (m_write_descriptor != FLUTE_INVALID_SOCKET && m_write_descriptor != m_read_descriptor) {
        flute::close(m_write_descriptor);
    }
    if (m_read_descriptor != FLUTE_INVALID_SOCKET) {
        flute::close(m_read_descriptor);
    }
    m_write_descriptor = m_read_descriptor = FLUTE_INVALID_SOCKET;
    if (m_channel) {
        m_channel->disableAll();
        delete m_channel;
        m_channel = nullptr;
    }
}

void EventLoopInterrupter::handleRead() {
    std::uint64_t num;
    flute::read(m_read_descriptor, &num, sizeof(num));
}

} // namespace flute