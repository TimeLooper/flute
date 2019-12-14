/*************************************************************************
 *
 * File Name:  Channel.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#pragma once

#include <flute/EventLoop.h>
#include <flute/config.h>
#include <flute/copyable.h>
#include <flute/socket_types.h>

#include <functional>

namespace flute {

class Channel : private copyable {
public:
    FLUTE_API_DECL Channel(socket_type descriptor, EventLoop* loop)
        : m_events(FileEvent::NONE), m_descriptor(descriptor), m_loop(loop), m_readCallback(), m_writeCallback() {
    }
    FLUTE_API_DECL ~Channel() = default;

    FLUTE_API_DECL void handleEvent(int events);
    FLUTE_API_DECL void disableRead();
    FLUTE_API_DECL void enableRead();
    FLUTE_API_DECL void disableWrite();
    FLUTE_API_DECL void enableWrite();
    FLUTE_API_DECL void disableAll();
    FLUTE_API_DECL void setReadCallback(const std::function<void()>& cb);
    FLUTE_API_DECL void setReadCallback(std::function<void()>&& cb);
    FLUTE_API_DECL void setWriteCallback(const std::function<void()>& cb);
    FLUTE_API_DECL void setWriteCallback(std::function<void()>&& cb);
    FLUTE_API_DECL socket_type descriptor() const;
    FLUTE_API_DECL int events() const;

private:
    int m_events;
    socket_type m_descriptor;
    EventLoop* m_loop;
    std::function<void()> m_readCallback;
    std::function<void()> m_writeCallback;

    void handleEventWithGuard(int events);
};

} // namespace flute