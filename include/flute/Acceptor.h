/*************************************************************************
 *
 * File Name:  Acceptor.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/16
 *
 *************************************************************************/

#pragma once

#include <flute/Logger.h>
#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

namespace flute {

class EventLoop;
class Channel;
class Socket;
class EventLoopGroup;
class InetAddress;

class Acceptor : private noncopyable {
public:
    FLUTE_API_DECL Acceptor(EventLoopGroup* loopGroup, const InetAddress& address, bool reusePort);
    FLUTE_API_DECL ~Acceptor();

    inline void setAcceptCallback(const AcceptCallback& cb) { m_acceptCallback = cb; }
    inline void setAcceptCallback(AcceptCallback&& cb) { m_acceptCallback = std::move(cb); }
    inline bool listening() const { return m_listening; }
    inline EventLoop* getEventLoop() const { return m_loop; }

    FLUTE_API_DECL void listen();
    FLUTE_API_DECL void close();

private:
    bool m_listening;
    socket_type m_idleDescriptor;
    Socket* m_socket;
    EventLoop* m_loop;
    Channel* m_channel;
    AcceptCallback m_acceptCallback;

    void handleRead();
};

} // namespace flute
