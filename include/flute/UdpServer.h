//
// Created by why on 2020/01/05.
//

#ifndef FLUTE_UDP_SERVER_H
#define FLUTE_UDP_SERVER_H

#include <flute/config.h>
#include <flute/noncopyable.h>

#include <memory>
#include <map>

namespace flute {

class EventLoopGroup;
class InetAddress;

class UdpServer : private noncopyable, public std::enable_shared_from_this<UdpServer> {
public:
    FLUTE_API_DECL explicit UdpServer(EventLoopGroup* eventLoopGroup);
    FLUTE_API_DECL ~UdpServer();

    FLUTE_API_DECL void bind(const InetAddress& address);
    FLUTE_API_DECL void close();

private:
    std::map<InetAddress, UdpConnection> m_connections;
};

} // namespace flute

#endif // FLUTE_UDP_SERVER_H
