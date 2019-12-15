/*************************************************************************
 *
 * File Name:  Channel.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include <flute/Channel.h>
#include <flute/EventLoop.h>

namespace flute {

void Channel::handleEvent(int events) {
    if (events & FileEvent::READ) {
        m_readCallback();
    }
    if (events & FileEvent::WRITE) {
        m_writeCallback();
    }
}

void Channel::disableRead() {
    if (m_events & FileEvent::READ) {
        m_loop->removeEvent(this, FileEvent::READ);
        m_events &= (~FileEvent::READ);
    }
}

void Channel::enableRead() {
    if (!(m_events & FileEvent::READ)) {
        m_loop->addEvent(this, FileEvent::READ);
        m_events |= FileEvent::READ;
    }
}

void Channel::disableWrite() {
    if (m_events & FileEvent::WRITE) {
        m_loop->removeEvent(this, FileEvent::WRITE);
        m_events &= (~FileEvent::WRITE);
    }
}

void Channel::enableWrite() {
    if (!(m_events & FileEvent::WRITE)) {
        m_loop->addEvent(this, FileEvent::WRITE);
        m_events |= FileEvent::WRITE;
    }
}

void Channel::disableAll() {
    if (m_events & (FileEvent::WRITE | FileEvent::READ)) {
        m_loop->removeEvent(this, m_events);
        m_events = FileEvent::NONE;
    }
}

void Channel::setReadCallback(const std::function<void()>& cb) {
    m_readCallback = cb;
}

void Channel::setReadCallback(std::function<void()>&& cb) {
    m_readCallback = std::move(cb);
}

void Channel::setWriteCallback(const std::function<void()>& cb) {
    m_writeCallback = cb;
}

void Channel::setWriteCallback(std::function<void()>&& cb) {
    m_writeCallback = std::move(cb);
}

socket_type Channel::descriptor() const {
    return m_descriptor;
}

int Channel::events() const {
    return m_events;
}

bool Channel::isWriteable() const {
    return m_events & FileEvent::WRITE;
}

bool Channel::isReadable() const {
    return m_events & FileEvent::READ;
}

} // namespace flute