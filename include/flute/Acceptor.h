//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_ACCEPTOR_H
#define FLUTE_ACCEPTOR_H

#include <flute/Logger.h>
#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

namespace flute {

class EventLoop;
class Channel;
class Socket;
class InetAddress;

class Acceptor : private noncopyable {
public:
    // FLUTE_API_DECL Acceptor(EventLoop* loop, const InetAddress& address, bool reusePort);
    FLUTE_API_DECL explicit Acceptor(EventLoop* loop);
    FLUTE_API_DECL ~Acceptor();

    inline void setAcceptCallback(const AcceptCallback& cb) { m_acceptCallback = cb; }
    inline void setAcceptCallback(AcceptCallback&& cb) { m_acceptCallback = std::move(cb); }
    inline bool listening() const { return m_listening; }

    FLUTE_API_DECL void bind(const InetAddress& address);
    FLUTE_API_DECL void setReusePort(bool reusePort);
    FLUTE_API_DECL void setReuseAddress(bool reuseAddress);
    FLUTE_API_DECL void listen();
    FLUTE_API_DECL void close();

private:
    bool m_listening;
    bool m_reuseAddress;
    bool m_reusePort;
    Socket* m_socket;
    EventLoop* m_loop;
    Channel* m_channel;
    AcceptCallback m_acceptCallback;

    void handleRead();
};

} // namespace flute

#endif // FLUTE_ACCEPTOR_H