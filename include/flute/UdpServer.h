//
// Created by why on 2020/01/05.
//

#ifndef FLUTE_UDP_SERVER_H
#define FLUTE_UDP_SERVER_H

#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <atomic>
#include <map>
#include <memory>

namespace flute {

class EventLoopGroup;
class InetAddress;
class Channel;
class Socket;

class UdpServer : private noncopyable, public std::enable_shared_from_this<UdpServer> {
public:
    FLUTE_API_DECL explicit UdpServer(EventLoopGroup* eventLoopGroup);

    FLUTE_API_DECL ~UdpServer();

    inline void setMessageCallback(UdpMessageCallback&& cb) { m_messageCallback = std::move(cb); }
    inline void setMessageCallback(const UdpMessageCallback& cb) { m_messageCallback = cb; }

    FLUTE_API_DECL void bind(const InetAddress& address);
    FLUTE_API_DECL flute::ssize_t send(const InetAddress& address, const void* buffer, flute::ssize_t length);
    FLUTE_API_DECL flute::ssize_t send(const InetAddress& address, const std::string& message);
    FLUTE_API_DECL flute::ssize_t send(const InetAddress& address, RingBuffer& buffer);
    FLUTE_API_DECL void close();

private:
    EventLoopGroup* m_eventLoopGroup;
    Channel* m_channel;
    Socket* m_socket;

    UdpMessageCallback m_messageCallback;

    void handleRead();
};

} // namespace flute

#endif // FLUTE_UDP_SERVER_H
