//
// Created by why on 2020/01/06.
//

#ifndef FLUTE_UDP_CONNECTION_H
#define FLUTE_UDP_CONNECTION_H

#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/InetAddress.h>

#include <memory>

namespace flute {

class UdpConnection : private noncopyable, public std::enable_shared_from_this<UdpConnection> {
public:
    FLUTE_API_DECL UdpConnection(const InetAddress& localAddress, const InetAddress& remoteAddress);
    FLUTE_API_DECL ~UdpConnection();

    FLUTE_API_DECL void receiveMessage(Buffer& buffer);

private:
    const InetAddress m_localAddress;
    const InetAddress m_remoteAddress;
};

} // namespace flute

#endif // FLUTE_UDP_CONNECTION_H