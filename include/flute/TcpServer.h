/*************************************************************************
 *
 * File Name:  TcpServer.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/16
 *
 *************************************************************************/

#pragma once

#include <flute/TcpConnection.h>
#include <flute/noncopyable.h>

namespace flute {

class EventLoopGroup;

class TcpServer : private noncopyable {
public:
    explicit TcpServer(EventLoopGroup* parent);
    TcpServer(EventLoopGroup* parent, EventLoopGroup* child);

    ~TcpServer();

    inline void setMessageCallback(MessageCallback&& cb) { m_messageCallback = std::move(cb); };
    inline void setMessageCallback(const MessageCallback& cb) { m_messageCallback = cb; };
    inline void setWriteCompleteCallback(WriteCompleteCallback&& cb) { m_writeCompleteCallback = std::move(cb); };
    inline void setWriteCompleteCallback(const WriteCompleteCallback& cb) { m_writeCompleteCallback = cb; };
    inline void setHighWaterMarkCallback(HighWaterMarkCallback&& cb) { m_highWaterMarkCallback = std::move(cb); };
    inline void setHighWaterMarkCallback(const HighWaterMarkCallback& cb) { m_highWaterMarkCallback = cb; };
    inline void setConnectionDestroyCallback(ConnectionDestroyCallback&& cb) {
        m_connectionDestroyCallback = std::move(cb);
    };
    inline void setConnectionDestroyCallback(const ConnectionDestroyCallback& cb) { m_connectionDestroyCallback = cb; };

private:
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    ConnectionDestroyCallback m_connectionDestroyCallback;
};

} // namespace flute
