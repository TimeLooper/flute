/*************************************************************************
 *
 * File Name:  TcpClient.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/19 23:56:41
 *
 *************************************************************************/

#pragma once

#include <flute/config.h>
#include <flute/InetAddress.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <mutex>
#include <future>

namespace flute {

class EventLoopGroup;
class Connector;

class TcpClient {
public:
    FLUTE_API_DECL explicit TcpClient(EventLoopGroup* loop, const InetAddress& address);
    FLUTE_API_DECL ~TcpClient();

    FLUTE_API_DECL std::future<void> connect();
    FLUTE_API_DECL void disconnect();
    FLUTE_API_DECL void stop();
    FLUTE_API_DECL void sync();

    inline void setMessageCallback(MessageCallback&& cb) { m_messageCallback = std::move(cb); }
    inline void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
    inline void setWriteCompleteCallback(WriteCompleteCallback&& cb) {
        m_writeCompleteCallback = std::move(cb);
    }
    inline void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }
    inline void setConnectionEstablishedCallback(ConnectionEstablishedCallback&& cb) {
        m_connectionEstablishedCallback = std::move(cb);
    }
    inline void setConnectionEstablishedCallback(const ConnectionEstablishedCallback& cb) {
        m_connectionEstablishedCallback = cb;
    }
    inline std::shared_ptr<TcpConnection> getConnection() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_connection;
    }
    inline bool retry() const { return m_retry; }
    inline void enableRetry() { m_retry = true; }

private:
    EventLoopGroup* m_loop;
    std::atomic<bool> m_retry;
    std::atomic<bool> m_connect;
    std::promise<void> m_connectPromise;
    std::promise<void> m_closePromise;
    mutable std::mutex m_mutex;
    std::shared_ptr<Connector> m_connector;
    std::shared_ptr<TcpConnection> m_connection;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    ConnectionEstablishedCallback m_connectionEstablishedCallback;

    void onConnected(socket_type descriptor);
    void removeConnection(const std::shared_ptr<TcpConnection>& connection);
};

} // namespace flute