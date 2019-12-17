/*************************************************************************
 *
 * File Name:  InetAddress.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/18
 *
 *************************************************************************/

#pragma once

#include <flute/config.h>
#include <flute/copyable.h>
#include <flute/socket_ops.h>

namespace flute {

class InetAddress : private copyable {
public:
    FLUTE_API_DECL explicit InetAddress(std::uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
    FLUTE_API_DECL explicit InetAddress(const char* ip, std::uint16_t port, bool ipv6 = false);
    FLUTE_API_DECL explicit InetAddress(const sockaddr_in& addr);
    FLUTE_API_DECL explicit InetAddress(const sockaddr_in6& addr);
    FLUTE_API_DECL ~InetAddress();

    FLUTE_API_DECL unsigned short int family() const;
    FLUTE_API_DECL const sockaddr* getSocketAddress() const;
    FLUTE_API_DECL void setScopeId(std::uint32_t scopeId);
    FLUTE_API_DECL std::string toString() const;
    FLUTE_API_DECL static bool resolve(const std::string& host, InetAddress* result);

private:
    union {
        struct sockaddr m_addr;
        struct sockaddr_in m_addr4;
        struct sockaddr_in6 m_addr6;
    };
};

} // namespace flute