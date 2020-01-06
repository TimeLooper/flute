//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_INETADDRESS_H
#define FLUTE_INETADDRESS_H

#include <flute/config.h>
#include <flute/copyable.h>
#include <flute/socket_ops.h>

namespace flute {

class InetAddress : private copyable {
public:
    FLUTE_API_DECL explicit InetAddress(std::uint16_t port = 0, bool loopbackOnly = false, bool ipv6 = false);
    FLUTE_API_DECL explicit InetAddress(const char* ip, std::uint16_t port, bool ipv6 = false);
    FLUTE_API_DECL InetAddress(const sockaddr_in& addr);
    FLUTE_API_DECL InetAddress(const sockaddr_in6& addr);
    FLUTE_API_DECL ~InetAddress() = default;

    inline unsigned short int family() const { return m_addr.sa_family; }
    inline const sockaddr* getSocketAddress() const { return &m_addr; }
    inline sockaddr* getSocketAddress() { return &m_addr; }
    inline const std::size_t getSocketLength() const { return sizeof(sockaddr_in6); }

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

#endif // FLUTE_INETADDRESS_H