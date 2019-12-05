/*************************************************************************
 *
 * File Name:  Connection.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/05 23:36:34
 *
 *************************************************************************/

#pragma once

#include <flute/noncopyable.h>
#include <flute/socket_types.h>
#include <flute/socket_ops.h>

#include <cstddef>

namespace flute {

class EventLoop;

class Connection : private noncopyable {
public:
    Connection(socket_type sockfd, EventLoop* loop, const sockaddr_storage& localAddress, const sockaddr_storage& remoteAddress);
    ~Connection();

private:
    socket_type m_sockfd;
    std::size_t m_highWaterMark;
    EventLoop* m_loop;
    const sockaddr_storage m_localAddress;
    const sockaddr_storage m_remoteAddress;
};

} // namespace flute