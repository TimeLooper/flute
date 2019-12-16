/*************************************************************************
 *
 * File Name:  Connection.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/05 23:36:34
 *
 *************************************************************************/

#pragma once

#include <flute/Buffer.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>
#include <flute/socket_ops.h>

#include <atomic>
#include <cstddef>
#include <memory>

namespace flute {

class EventLoop;
class Channel;

class Connection : private noncopyable, public std::enable_shared_from_this<Connection> {
public:
    typedef std::function<void(const std::shared_ptr<Connection>&, Buffer&)> MessageCallback;
    typedef std::function<void(const std::shared_ptr<Connection>&)> CloseCallback;
    typedef std::function<void(const std::shared_ptr<Connection>&)> WriteCompleteCallback;
    typedef std::function<void(const std::shared_ptr<Connection>&, flute::ssize_t)> HighWaterMarkCallback;

    FLUTE_API_DECL Connection(socket_type descriptor, EventLoop* loop, const sockaddr_storage& localAddress,
                              const sockaddr_storage& remoteAddress);
    FLUTE_API_DECL ~Connection();

    FLUTE_API_DECL void setMessageCallback(const MessageCallback& cb);
    FLUTE_API_DECL void setMessageCallback(MessageCallback&& cb);
    FLUTE_API_DECL void setCloseCallback(const CloseCallback& cb);
    FLUTE_API_DECL void setCloseCallback(CloseCallback&& cb);
    FLUTE_API_DECL void setWriteCompleteCallback(const WriteCompleteCallback& cb);
    FLUTE_API_DECL void setWriteCompleteCallback(WriteCompleteCallback&& cb);
    FLUTE_API_DECL void setHighWaterMarkCallback(const HighWaterMarkCallback& cb);
    FLUTE_API_DECL void setHighWaterMarkCallback(HighWaterMarkCallback&& cb);
    FLUTE_API_DECL socket_type descriptor() const;
    FLUTE_API_DECL EventLoop* getEventLoop() const;
    FLUTE_API_DECL const sockaddr_storage& getLocalAddress() const;
    FLUTE_API_DECL const sockaddr_storage& getRemoteAddress() const;
    FLUTE_API_DECL int shutdown(int flags) const;
    FLUTE_API_DECL void send(const void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void send(const std::string& message);
    FLUTE_API_DECL void send(Buffer& buffer);

private:
    enum ConnectionState { DISCONNECTED, CONNECTING, CONNECTED, DISCONNECTING };
    socket_type m_descriptor;
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
};

} // namespace flute