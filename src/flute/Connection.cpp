/*************************************************************************
 *
 * File Name:  Connection.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/05 23:46:29
 *
 *************************************************************************/

#include <flute/Connection.h>

namespace flute {

Connection::Connection(socket_type sockfd, EventLoop* loop, const sockaddr_storage& localAddress, const sockaddr_storage& remoteAddress)
    : m_sockfd(sockfd)
    , m_highWaterMark(0)
    , m_loop(loop)
    , m_localAddress(localAddress)
    , m_remoteAddress(remoteAddress) {

}

Connection::~Connection() {

}

} // namespace flute