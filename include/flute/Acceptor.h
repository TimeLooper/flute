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

class Acceptor : private noncopyable {
public:
    Acceptor(EventLoop* loop, const sockaddr_storage& address, bool reusePort);
    ~Acceptor();

    inline void setAcceptCallback(const AcceptCallback& cb) { m_acceptCallback = cb; }
    inline void setAcceptCallback(AcceptCallback&& cb) { m_acceptCallback = std::move(cb); }
    inline bool listening() const { return m_listening; }

    void listen();
    void close();

private:
    bool m_listening;
    socket_type m_idleDescriptor;
    EventLoop* m_loop;
    Socket* m_socket;
    Channel* m_channel;
    AcceptCallback m_acceptCallback;

    void handleRead();
};

} // namespace flute
