//
// Created by why on 2019/12/30.
//

#include <flute/Channel.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopInterruptor.h>
#include <flute/Logger.h>
#include <flute/flute-config.h>
#include <flute/internal.h>
#include <flute/socket_ops.h>

#include <functional>

namespace flute {

EventLoopInterruptor::EventLoopInterruptor(EventLoop *loop)
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
        flute::closeSocket(m_writeDescriptor);
        m_writeDescriptor = FLUTE_INVALID_SOCKET;
    }
    if (m_readDescriptor != FLUTE_INVALID_SOCKET) {
        flute::closeSocket(m_readDescriptor);
        m_readDescriptor = FLUTE_INVALID_SOCKET;
    }
    delete m_channel;
}

void EventLoopInterruptor::interrupt() const {
    if (m_writeDescriptor == FLUTE_INVALID_SOCKET) {
        return;
    }
    std::uint64_t num = 1;
#ifdef _WIN32
    flute::send(m_writeDescriptor, reinterpret_cast<char *>(&num), sizeof(num), 0);
#else
    flute::write(m_writeDescriptor, &num, sizeof(num));
#endif
}

void EventLoopInterruptor::handleRead() {
    std::uint64_t num;
#ifdef _WIN32
    flute::recv(m_readDescriptor, reinterpret_cast<char *>(&num), sizeof(num), 0);
#else
    flute::read(m_readDescriptor, &num, sizeof(num));
#endif
}

} // namespace flute