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

}

void Channel::disableRead() {
    if (m_events & FileEvent::READ) {
        m_loop->removeEvent(this, FileEvent::READ);
    }
}

void Channel::enableRead() {

}

void Channel::disableWrite() {

}

void Channel::enableWrite() {

}

void Channel::disableAll() {

}

} // namespace flute