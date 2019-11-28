/*************************************************************************
 *
 * File Name:  EventLoop.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include <flute/EventLoop.h>
#include <flute/Channel.h>
#include <flute/Reactor.h>

namespace flute {

EventLoop::EventLoop() {
}

EventLoop::~EventLoop() {
}

void EventLoop::addEvent(Channel* channel, int events) {
    m_reactor->add(channel->descriptor(), channel->events(), events, channel);
}

void EventLoop::removeEvent(Channel* channel, int events) {
    m_reactor->remove(channel->descriptor(), channel->events(), events, channel);
}

} // namespace flute