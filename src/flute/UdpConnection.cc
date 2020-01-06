//
// Created by why on 2020/01/06.
//

#include <flute/UdpConnection.h>

namespace flute {

UdpConnection::UdpConnection(const InetAddress& localAddress, const InetAddress& remoteAddress) : m_localAddress(localAddress), m_remoteAddress(remoteAddress) {

}

} // namespace flute