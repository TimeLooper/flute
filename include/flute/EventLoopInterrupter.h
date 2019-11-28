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
    FLUTE_API_DECL EventLoopInterrupter(EventLoop* loop);
    FLUTE_API_DECL ~EventLoopInterrupter();

    void interrupt();

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