//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_EVENT_LOOP_INTERRUPTOR_H
#define FLUTE_EVENT_LOOP_INTERRUPTOR_H

#include <flute/flute_types.h>
#include <flute/noncopyable.h>

namespace flute {

class EventLoop;
class Channel;

class EventLoopInterruptor : private noncopyable {
public:
    explicit EventLoopInterruptor(EventLoop* loop);
    ~EventLoopInterruptor();

    void interrupt() const;
    inline socket_type readDescriptor() const { return m_readDescriptor; }

private:
    socket_type m_readDescriptor;
    socket_type m_writeDescriptor;
    EventLoop* m_loop;
    Channel* m_channel;

    void handleRead();
};

} // namespace flute

#endif // FLUTE_EVENT_LOOP_INTERRUPTOR_H