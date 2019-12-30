//
// Created by why on 2019/12/30.
//

#include <flute/Channel.h>
#include <flute/EventLoop.h>

#include <cassert>

namespace flute {

Channel::Channel(socket_type descriptor, EventLoop* loop)
    : m_events(FluteEvent::NONE), m_descriptor(descriptor), m_loop(loop), m_readCallback(), m_writeCallback() {}

Channel::~Channel() { assert(m_events == FluteEvent::NONE); }

void Channel::handleEvent(int events) {
    if ((events & FluteEvent::READ) && m_readCallback) {
        m_readCallback();
    }
    if ((events & FluteEvent::WRITE) && m_writeCallback) {
        m_writeCallback();
    }
}

void Channel::disableRead() {
    if (m_events & FluteEvent::READ) {
        m_loop->removeEvent(this, FluteEvent::READ);
        m_events &= (~FluteEvent::READ);
    }
}

void Channel::enableRead() {
    if (!(m_events & FluteEvent::READ)) {
        m_loop->addEvent(this, FluteEvent::READ);
        m_events |= FluteEvent::READ;
    }
}

void Channel::disableWrite() {
    if (m_events & FluteEvent::WRITE) {
        m_loop->removeEvent(this, FluteEvent::WRITE);
        m_events &= (~FluteEvent::WRITE);
    }
}

void Channel::enableWrite() {
    if (!(m_events & FluteEvent::WRITE)) {
        m_loop->addEvent(this, FluteEvent::WRITE);
        m_events |= FluteEvent::WRITE;
    }
}

void Channel::disableAll() {
    if (m_events & (FluteEvent::READ | FluteEvent::WRITE)) {
        m_loop->removeEvent(this, m_events);
        m_events = FluteEvent::NONE;
    }
}

} // namespace flute