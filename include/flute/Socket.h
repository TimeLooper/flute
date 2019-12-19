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
    FLUTE_API_DECL explicit Socket(socket_type descriptor);
    FLUTE_API_DECL ~Socket();

    FLUTE_API_DECL void bind(const InetAddress& address);
    FLUTE_API_DECL void listen();
    inline socket_type descriptor() const { return m_descriptor; }
    inline int getSocketError() const { return flute::getSocketError(m_descriptor); }
    FLUTE_API_DECL socket_type accept();
    FLUTE_API_DECL void setTcpNoDelay(bool on);
    FLUTE_API_DECL void setReuseAddress(bool on);
    FLUTE_API_DECL void setReusePort(bool on);
    FLUTE_API_DECL void setKeepAlive(bool on);
    FLUTE_API_DECL void shutdownWrite();
    FLUTE_API_DECL void shutdownRead();
    FLUTE_API_DECL void close();

private:
    socket_type m_descriptor;
};

} // namespace flute
