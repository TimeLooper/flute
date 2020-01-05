//
// Created by why on 2020/01/05.
//

#ifndef FLUTE_TCP_CLIENT_H
#define FLUTE_TCP_CLIENT_H

#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/flute_types.h>

#include <memory>
#include <atomic>
#include <mutex>

namespace flute {

class EventLoopGroup;
class EventLoop;
class Connector;
class TcpConnection;
class InetAddress;

class TcpClient : private noncopyable, public std::enable_shared_from_this<TcpClient> {
public:
    FLUTE_API_DECL TcpClient(EventLoopGroup* loopGroup, const InetAddress& address);
    FLUTE_API_DECL TcpClient(EventLoopGroup* loopGroup, InetAddress&& address);
    FLUTE_API_DECL ~TcpClient();

    FLUTE_API_DECL void connect();
    FLUTE_API_DECL void disconnect();
    FLUTE_API_DECL void stop();

    inline void setConnectionEstablishedCallback(ConnectionEstablishedCallback&& callback) { m_connectionEstablishedCallback = std::move(callback); }
    inline void setConnectionEstablishedCallback(const ConnectionEstablishedCallback& callback) { m_connectionEstablishedCallback = callback; }
    inline void setMessageCallback(MessageCallback&& callback) { m_messageCallback = std::move(callback); }
    inline void setMessageCallback(const MessageCallback& callback) { m_messageCallback = callback; }
    inline void setWriteCompleteCallback(WriteCompleteCallback&& callback) { m_writeCompleteCallback = std::move(callback); }
    inline void setWriteCompleteCallback(const WriteCompleteCallback& callback) { m_writeCompleteCallback = callback; }

private:
    EventLoopGroup* m_loopGroup;
    std::atomic<bool> m_retry;
    std::atomic<bool> m_isConnect;
    mutable std::mutex m_mutex;
    std::shared_ptr<Connector> m_connector;
    std::shared_ptr<TcpConnection> m_connection;
    ConnectionEstablishedCallback m_connectionEstablishedCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;

    void onConnectSuccess(socket_type descriptor);
    void removeConnection(const std::shared_ptr<TcpConnection>& connection);
};

} // namespace flute

#endif // FLUTE_TCP_CLIENT_H