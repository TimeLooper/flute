/*************************************************************************
 *
 * File Name:  TcpConnection.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/05
 *
 *************************************************************************/

#pragma once

#include <flute/Buffer.h>
#include <flute/Channel.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>
#include <flute/socket_ops.h>

#include <atomic>
#include <cstddef>

namespace flute {

class EventLoop;

class TcpConnection : private noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    FLUTE_API_DECL TcpConnection(socket_type descriptor, EventLoop* loop, const sockaddr_storage& localAddress,
                                 const sockaddr_storage& remoteAddress);
    FLUTE_API_DECL ~TcpConnection();

    FLUTE_API_DECL int shutdown(int flags) const;
    FLUTE_API_DECL void send(const void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void send(const std::string& message);
    FLUTE_API_DECL void send(Buffer& buffer);
    FLUTE_API_DECL void handleConnectionEstablished();
    FLUTE_API_DECL void handleConnectionDestroy();

    inline void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; }
    inline void setMessageCallback(MessageCallback&& cb) { m_messageCallback = std::move(cb); }
    inline void setCloseCallback(const CloseCallback& cb) { m_closeCallback = cb; }
    inline void setCloseCallback(CloseCallback&& cb) { m_closeCallback = std::move(cb); }
    inline void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; }
    inline void setWriteCompleteCallback(WriteCompleteCallback&& cb) { m_writeCompleteCallback = std::move(cb); }
    inline void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { m_highWaterMarkCallback = cb; }
    inline void setHighWaterMarkCallback(HighWaterMarkCallback&& cb) { m_highWaterMarkCallback = std::move(cb); }
    inline void setConnectionEstablishedCallback(const ConnectionEstablishedCallback& cb) {
        m_connectionEstablishedCallback = cb;
    }
    inline void setConnectionEstablishedCallback(ConnectionEstablishedCallback&& cb) {
        m_connectionEstablishedCallback = std::move(cb);
    }
    inline void setConnectionDestroyCallback(const ConnectionDestroyCallback& cb) { m_connectionDestroyCallback = cb; }
    inline void setConnectionDestroyCallback(ConnectionDestroyCallback&& cb) {
        m_connectionDestroyCallback = std::move(cb);
    }
    inline socket_type descriptor() const { return m_channel ? m_channel->descriptor() : FLUTE_INVALID_SOCKET; }
    inline EventLoop* getEventLoop() const { return m_loop; }
    inline const sockaddr_storage& getLocalAddress() const { return m_localAddress; }
    inline const sockaddr_storage& getRemoteAddress() const { return m_remoteAddress; }
    inline bool connected() const { return m_state == ConnectionState::CONNECTED; }

private:
    enum ConnectionState { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING };
    flute::ssize_t m_highWaterMark;
    EventLoop* m_loop;
    std::atomic<ConnectionState> m_state;
    std::unique_ptr<Channel> m_channel;
    const sockaddr_storage m_localAddress;
    const sockaddr_storage m_remoteAddress;
    MessageCallback m_messageCallback;
    CloseCallback m_closeCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    ConnectionEstablishedCallback m_connectionEstablishedCallback;
    ConnectionDestroyCallback m_connectionDestroyCallback;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void shutdownInLoop();
    void setTcpNoDelay(bool on) const;
    void sendInLoop(const void* buffer, flute::ssize_t length);
    void sendInLoop(const std::string& message);
    void sendInLoop(Buffer& buffer);
    void handleConnectionEstablishedInLoop();
    void handleConnectionDestroyInLoop();
};

} // namespace flute