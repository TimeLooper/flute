//
// Created by why on 2019/12/30.
//

#include <flute/Channel.h>
#include <flute/EventLoop.h>

#include <cassert>

namespace flute {

Channel::Channel(socket_type descriptor, EventLoop* loop)
    : m_events(SelectorEvent::EVENT_NONE)
    , m_descriptor(descriptor)
    , m_loop(loop)
    , m_readCallback()
    , m_writeCallback() {}

Channel::~Channel() { assert(m_events == SelectorEvent::EVENT_NONE); }

void Channel::handleEvent(int events) {
    if ((events & SelectorEvent::EVENT_READ) && m_readCallback) {
        m_readCallback();
    }
    if ((events & SelectorEvent::EVENT_WRITE) && m_writeCallback) {
        m_writeCallback();
    }
}

void Channel::disableRead() {
    if (m_events & SelectorEvent::EVENT_READ) {
        m_loop->removeEvent(this, SelectorEvent::EVENT_READ);
        m_events &= (~SelectorEvent::EVENT_READ);
    }
}

void Channel::enableRead() {
    if (!(m_events & SelectorEvent::EVENT_READ)) {
        m_loop->addEvent(this, SelectorEvent::EVENT_READ);
        m_events |= SelectorEvent::EVENT_READ;
    }
}

void Channel::disableWrite() {
    if (m_events & SelectorEvent::EVENT_WRITE) {
        m_loop->removeEvent(this, SelectorEvent::EVENT_WRITE);
        m_events &= (~SelectorEvent::EVENT_WRITE);
    }
}

void Channel::enableWrite() {
    if (!(m_events & SelectorEvent::EVENT_WRITE)) {
        m_loop->addEvent(this, SelectorEvent::EVENT_WRITE);
        m_events |= SelectorEvent::EVENT_WRITE;
    }
}

void Channel::disableAll() {
    if (m_events & (SelectorEvent::EVENT_READ | SelectorEvent::EVENT_WRITE)) {
        m_loop->removeEvent(this, m_events);
        m_events = SelectorEvent::EVENT_NONE;
    }
}

} // namespace flute