/*************************************************************************
 *
 * File Name:  EventLoopInterrupter.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29 00:58:36
 *
 *************************************************************************/

#pragma once
#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/socket_types.h>

namespace flute {
class EventLoop;
class Channel;

class EventLoopInterrupter : private noncopyable {
public:
    EventLoopInterrupter(EventLoop* loop);
    ~EventLoopInterrupter();

    void interrupt();
    inline socket_type readDescriptor() const {
        return m_read_descriptor;
    }

private:
    socket_type m_read_descriptor;
    socket_type m_write_descriptor;
    Channel* m_channel;
    EventLoop* m_loop;

    void open();
    void close();
    void handleRead();
};

} // namespace flute