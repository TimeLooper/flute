//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_TCP_CONNECTION_H
#define FLUTE_TCP_CONNECTION_H

#include <flute/AsyncIoService.h>
#include <flute/InetAddress.h>
#include <flute/RingBuffer.h>
#include <flute/Socket.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>
#include <flute/socket_ops.h>

#include <atomic>
#include <cstddef>
#include <memory>

namespace flute {

class EventLoop;
class Channel;

class TcpConnection : private noncopyable, public std::enable_shared_from_this<TcpConnection> {
public:
    FLUTE_API_DECL TcpConnection(socket_type descriptor, EventLoop* loop, const InetAddress& localAddress,
                                 const InetAddress& remoteAddress, bool isClient = false);
    FLUTE_API_DECL ~TcpConnection();

    FLUTE_API_DECL void shutdown();
    FLUTE_API_DECL void send(const void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void send(const std::string& message);
    FLUTE_API_DECL void send(std::string&& message);
    FLUTE_API_DECL void handleConnectionEstablished();
    FLUTE_API_DECL void handleConnectionDestroy();
    FLUTE_API_DECL void startRead();
    FLUTE_API_DECL void stopRead();
    FLUTE_API_DECL void startWrite();
    FLUTE_API_DECL void stopWrite();
    FLUTE_API_DECL void forceClose();

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
    inline socket_type descriptor() const { return m_socket ? m_socket->descriptor() : FLUTE_INVALID_SOCKET; }
    inline EventLoop* getEventLoop() const { return m_loop; }
    inline const InetAddress& getLocalAddress() const { return m_localAddress; }
    inline const InetAddress& getRemoteAddress() const { return m_remoteAddress; }
    inline bool connected() const { return m_state == ConnectionState::CONNECTED; }

private:
    enum class ConnectionState { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING };
    flute::ssize_t m_highWaterMark;
    EventLoop* m_loop;
    AsyncIoContext* m_readAsyncIoContext;
    AsyncIoContext* m_writeAsyncIoContext;
    std::atomic<ConnectionState> m_state;
    std::atomic<bool> m_reading;
    std::atomic<bool> m_writing;
    std::unique_ptr<Channel> m_channel;
    std::unique_ptr<Socket> m_socket;
    const InetAddress m_localAddress;
    const InetAddress m_remoteAddress;
    MessageCallback m_messageCallback;
    CloseCallback m_closeCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    ConnectionEstablishedCallback m_connectionEstablishedCallback;
    ConnectionDestroyCallback m_connectionDestroyCallback;
    RingBuffer m_inputBuffer;
    RingBuffer m_outputBuffer;
    RingBuffer m_asyncIoWriteBuffer;

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void shutdownInLoop();
    void sendInLoop(const void* buffer, flute::ssize_t length);
    void sendInLoop(std::string message);
    void handleConnectionEstablishedInLoop();
    void handleConnectionDestroyInLoop();
    void forceCloseInLoop();
    void handleAsyncIoComplete(AsyncIoCode code, ssize_t bytes, AsyncIoContext* ioContext);
    void handleAsyncIoCompleteInLoop(AsyncIoCode code, ssize_t bytes, AsyncIoContext* ioContext);
};

} // namespace flute

#endif // FLUTE_TCP_CONNECTION_H