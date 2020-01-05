//
// Created by why on 2020/01/05.
//

#ifndef FLUTE_UDP_CONNECTION_H
#define FLUTE_UDP_CONNECTION_H

#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/flute_types.h>
#include <flute/Buffer.h>
#include <flute/InetAddress.h>
#include <flute/Socket.h>

#include <memory>

namespace flute {

class EventLoop;
class EventLoopGroup;
class Channel;

class UdpConnection : private noncopyable, public std::enable_shared_from_this<UdpConnection> {
public:
    FLUTE_API_DECL UdpConnection(socket_type descriptor, EventLoop* loop, const InetAddress& localAddress, const InetAddress& remoteAddress);
    FLUTE_API_DECL ~UdpConnection();

    FLUTE_API_DECL void send(const void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void send(const std::string& message);
    FLUTE_API_DECL void send(Buffer& buffer);
    FLUTE_API_DECL void handleConnectionEstablished();
    FLUTE_API_DECL void handleConnectionDestroy();
    FLUTE_API_DECL void startRead();
    FLUTE_API_DECL void stopRead();
    FLUTE_API_DECL void startWrite();
    FLUTE_API_DECL void stopWrite();

    inline void setMessageCallback(const UdpMessageCallback& cb) { m_messageCallback = cb; }
    inline void setMessageCallback(UdpMessageCallback&& cb) { m_messageCallback = std::move(cb); }
    inline void setCloseCallback(const UdpCloseCallback& cb) { m_closeCallback = cb; }
    inline void setCloseCallback(UdpCloseCallback&& cb) { m_closeCallback = std::move(cb); }
    inline void setConnectionEstablishedCallback(const UdpConnectionEstablishedCallback& cb) {
        m_connectionEstablishedCallback = cb;
    }
    inline void setConnectionEstablishedCallback(UdpConnectionEstablishedCallback&& cb) {
        m_connectionEstablishedCallback = std::move(cb);
    }
    inline void setConnectionDestroyCallback(const UdpConnectionDestroyCallback& cb) { m_connectionDestroyCallback = cb; }
    inline void setConnectionDestroyCallback(UdpConnectionDestroyCallback&& cb) {
        m_connectionDestroyCallback = std::move(cb);
    }
    inline socket_type descriptor() const { return m_socket ? m_socket->descriptor() : FLUTE_INVALID_SOCKET; }
    inline EventLoop* getEventLoop() const { return m_loop; }
    inline const InetAddress& getLocalAddress() const { return m_localAddress; }
    inline const InetAddress& getRemoteAddress() const { return m_remoteAddress; }
    inline bool connected() const { return m_state == ConnectionState::CONNECTED; }

private:
    enum ConnectionState { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING };
    flute::ssize_t m_highWaterMark;
    EventLoop* m_loop;
    std::atomic<ConnectionState> m_state;
    std::unique_ptr<Channel> m_channel;
    std::unique_ptr<Socket> m_socket;
    const InetAddress m_localAddress;
    const InetAddress m_remoteAddress;
    UdpMessageCallback m_messageCallback;
    UdpCloseCallback m_closeCallback;
    UdpConnectionEstablishedCallback m_connectionEstablishedCallback;
    UdpConnectionDestroyCallback m_connectionDestroyCallback;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;

    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void shutdownInLoop();
    void sendInLoop(const void* buffer, flute::ssize_t length);
    void sendInLoop(const std::string& message);
    void sendInLoop(Buffer& buffer);
    void handleConnectionEstablishedInLoop();
    void handleConnectionDestroyInLoop();
    void forceCloseInLoop();
};

} // namespace flute

#endif // FLUTE_UDP_CONNECTION_H
