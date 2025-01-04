//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_CHANNEL_H
#define FLUTE_CHANNEL_H

#include <flute/EventLoop.h>
#include <flute/Selector.h>
#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <functional>

namespace flute {

class Channel : private noncopyable {
public:
    FLUTE_API_DECL Channel(socket_type descriptor, EventLoop* loop, bool autoUseAsyncIo = false);
    FLUTE_API_DECL ~Channel();

    FLUTE_API_DECL void handleEvent(int events);
    FLUTE_API_DECL void disableRead();
    FLUTE_API_DECL void enableRead();
    FLUTE_API_DECL void disableWrite();
    FLUTE_API_DECL void enableWrite();
    FLUTE_API_DECL void disableAll();

    inline void setReadCallback(const std::function<void()>& cb) { m_readCallback = cb; }
    inline void setReadCallback(std::function<void()>&& cb) { m_readCallback = std::move(cb); }
    inline void setWriteCallback(const std::function<void()>& cb) { m_writeCallback = cb; }
    inline void setWriteCallback(std::function<void()>&& cb) { m_writeCallback = std::move(cb); }
    inline socket_type descriptor() const { return m_descriptor; }
    inline int events() const { return m_events; }
    inline bool isWriteable() const { return m_events & SelectorEvent::EVENT_WRITE; }
    inline bool isReadable() const { return m_events & SelectorEvent::EVENT_READ; }

private:
    bool m_autoUseAsyncIo;
    int m_events;
    socket_type m_descriptor;
    EventLoop* m_loop;
    std::function<void()> m_readCallback;
    std::function<void()> m_writeCallback;
};

} // namespace flute

#endif // FLUTE_CHANNEL_H