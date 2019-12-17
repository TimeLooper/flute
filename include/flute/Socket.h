/*************************************************************************
 *
 * File Name:  Socket.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/17
 *
 *************************************************************************/

#pragma once

#include <flute/InetAddress.h>
#include <flute/flute_types.h>
#include <flute/socket_ops.h>

namespace flute {

class Socket {
public:
    explicit Socket(socket_type descriptor);
    ~Socket();

    void bind(const InetAddress& address);
    void listen();
    inline socket_type descriptor() const { return m_descriptor; }
    inline int getSocketError() const { return flute::getSocketError(m_descriptor); }
    socket_type accept();
    void setTcpNoDelay(bool on);
    void setReuseAddress(bool on);
    void setReusePort(bool on);
    void setKeepAlive(bool on);
    void shutdownWrite();
    void shutdownRead();
    void close();

private:
    socket_type m_descriptor;
};

} // namespace flute
