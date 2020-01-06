//
// Created by why on 2020/01/05.
//

#ifndef FLUTE_UDP_SERVER_H
#define FLUTE_UDP_SERVER_H

#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/flute_types.h>

#include <memory>
#include <map>
#include <atomic>

namespace flute {

class EventLoopGroup;
class InetAddress;
class UdpConnection;

class UdpServer : private noncopyable, public std::enable_shared_from_this<UdpServer> {
public:
    inline void setMessageCallback(UdpMessageCallback&& cb) { m_messageCallback = std::move(cb); }
    inline void setMessageCallback(const UdpMessageCallback& cb) { m_messageCallback = cb; }
    inline void setConnectionDestroyCallback(UdpConnectionDestroyCallback&& cb) {
        m_connectionDestroyCallback = std::move(cb);
    }
    inline void setConnectionDestroyCallback(const UdpConnectionDestroyCallback& cb) { m_connectionDestroyCallback = cb; }
    inline void setCloseCallback(UdpCloseCallback&& cb) { m_closeCallback = std::move(cb); }
    inline void setCloseCallback(const UdpCloseCallback& cb) { m_closeCallback = cb; }
    inline void setConnectionEstablishedCallback(UdpConnectionEstablishedCallback&& cb) {
        m_connectionEstablishedCallback = std::move(cb);
    }
    inline void setConnectionEstablishedCallback(const UdpConnectionEstablishedCallback& cb) {
        m_connectionEstablishedCallback = cb;
    }

    FLUTE_API_DECL void bind(const InetAddress& address);
    FLUTE_API_DECL void close();

private:
    enum ServerState { STOPPED, STARTING, STARTED, STOPPING };
    EventLoopGroup* m_eventLoopGroup;
    std::atomic<ServerState> m_state;
    // std::unique_ptr<Acceptor> m_acceptor;
    std::map<flute::socket_type, std::shared_ptr<UdpConnection>> m_connections;
    std::map<InetAddress, std::shared_ptr<UdpConnection>> m_connections;

    UdpMessageCallback m_messageCallback;
    UdpCloseCallback m_closeCallback;
    UdpConnectionEstablishedCallback m_connectionEstablishedCallback;
    UdpConnectionDestroyCallback m_connectionDestroyCallback;

    void handleConnectionClose(const std::shared_ptr<UdpConnection>& conn);
    void handleAcceptConnection(socket_type descriptor);
    void handleConnectionCloseInLoop(const std::shared_ptr<UdpConnection>& conn);
    void handleConnectionDestroy(const std::shared_ptr<UdpConnection>& conn);
    void handleConnectionDestroyInLoop(const std::shared_ptr<UdpConnection>& conn);
};

} // namespace flute

#endif // FLUTE_UDP_SERVER_H
