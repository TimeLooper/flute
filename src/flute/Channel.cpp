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

Channel::Channel(socket_type descriptor, EventLoop* loop)
    : m_events(FileEvent::NONE), m_descriptor(descriptor), m_loop(loop), m_readCallback(), m_writeCallback() {}

Channel::~Channel() { disableAll(); }

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

} // namespace flute