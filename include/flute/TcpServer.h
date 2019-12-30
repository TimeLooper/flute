//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_TCP_SERVER_H
#define FLUTE_TCP_SERVER_H

#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <atomic>
#include <future>
#include <map>
#include <memory>

namespace flute {

class Acceptor;
class EventLoopGroup;
class InetAddress;

class TcpServer : private noncopyable, public std::enable_shared_from_this<TcpServer> {
public:
    FLUTE_API_DECL explicit TcpServer(EventLoopGroup* eventLoopGroup);

    FLUTE_API_DECL ~TcpServer();

    inline void setMessageCallback(MessageCallback&& cb) { m_messageCallback = std::move(cb); }
    inline void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
    inline void setWriteCompleteCallback(WriteCompleteCallback&& cb) { m_writeCompleteCallback = std::move(cb); }
    inline void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; };
    inline void setHighWaterMarkCallback(HighWaterMarkCallback&& cb) { m_highWaterMarkCallback = std::move(cb); }
    inline void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { m_highWaterMarkCallback = cb; }
    inline void setConnectionDestroyCallback(ConnectionDestroyCallback&& cb) {
        m_connectionDestroyCallback = std::move(cb);
    }
    inline void setConnectionDestroyCallback(const ConnectionDestroyCallback& cb) { m_connectionDestroyCallback = cb; }
    inline void setCloseCallback(CloseCallback&& cb) { m_closeCallback = std::move(cb); }
    inline void setCloseCallback(const CloseCallback& cb) { m_closeCallback = cb; }
    inline void setConnectionEstablishedCallback(ConnectionEstablishedCallback&& cb) {
        m_connectionEstablishedCallback = std::move(cb);
    }
    inline void setConnectionEstablishedCallback(const ConnectionEstablishedCallback& cb) {
        m_connectionEstablishedCallback = cb;
    }

    FLUTE_API_DECL void bind(const InetAddress& address);
    FLUTE_API_DECL void close();

private:
    enum ServerState { STOPPED, STARTING, STARTED, STOPPING };
    EventLoopGroup* m_eventLoopGroup;
    std::atomic<ServerState> m_state;
    std::unique_ptr<Acceptor> m_acceptor;
    std::map<flute::socket_type, std::shared_ptr<TcpConnection>> m_connections;
    std::promise<void> m_serverPromise;

    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    ConnectionDestroyCallback m_connectionDestroyCallback;
    ConnectionEstablishedCallback m_connectionEstablishedCallback;
    CloseCallback m_closeCallback;

    void handleConnectionClose(const std::shared_ptr<TcpConnection>& conn);
    void handleAcceptConnection(socket_type descriptor);
    void handleConnectionCloseInLoop(const std::shared_ptr<TcpConnection>& conn);
    void handleConnectionDestroy(const std::shared_ptr<TcpConnection>& conn);
    void handleConnectionDestroyInLoop(const std::shared_ptr<TcpConnection>& conn);
};

} // namespace flute

#endif // FLUTE_TCP_SERVER_H