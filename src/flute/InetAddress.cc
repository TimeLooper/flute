//
// Created by why on 2019/12/30.
//

#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/endian.h>

#include <cassert>
#include <cstring>

namespace flute {

InetAddress::InetAddress(std::uint16_t port, bool loopbackOnly, bool ipv6) : m_addr6() {
    if (ipv6) {
        std::memset(&m_addr6, 0, sizeof(m_addr6));
        m_addr6.sin6_family = AF_INET6;
        in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
        m_addr6.sin6_addr = ip;
        m_addr6.sin6_port = flute::host2Network(port);
    } else {
        std::memset(&m_addr4, 0, sizeof(m_addr4));
        m_addr4.sin_family = AF_INET;
        auto ip = loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY;
        m_addr4.sin_addr.s_addr = flute::host2Network(static_cast<std::uint32_t>(ip));
        m_addr4.sin_port = flute::host2Network(port);
    }
}

InetAddress::InetAddress(const char* ip, const std::uint16_t port, bool ipv6) {
    if (ipv6) {
        std::memset(&m_addr6, 0, sizeof(m_addr6));
        flute::fromIpPort(ip, port, &m_addr6);
    } else {
        std::memset(&m_addr4, 0, sizeof(m_addr4));
        flute::fromIpPort(ip, port, &m_addr4);
    }
}

InetAddress::InetAddress(const sockaddr_in& addr) : m_addr4(addr) {}

InetAddress::InetAddress(const sockaddr_in6& addr) : m_addr6(addr) {}

void InetAddress::setScopeId(std::uint32_t scopeId) {
    if (m_addr.sa_family == AF_INET6) {
        m_addr6.sin6_scope_id = scopeId;
    }
}

std::string InetAddress::toString() const {
    char buffer[INET6_ADDRSTRLEN] = {0};
    flute::toIpPort(&m_addr, buffer, sizeof(buffer));
    return buffer;
}

bool InetAddress::resolve(const std::string& host, InetAddress* result) {
    assert(result != nullptr);
    addrinfo hints{}, *res = nullptr;

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    auto ret = getaddrinfo(host.c_str(), nullptr, &hints, &res);
    if (ret != 0) {
        LOG_ERROR << "host " << host << " getaddrinfo error " << ret << ":" << gai_strerror(ret) << ".";
        return false;
    }
    auto temp = res;
    if (temp->ai_family == AF_INET) {
        result->m_addr4.sin_addr.s_addr = reinterpret_cast<sockaddr_in*>(temp->ai_addr)->sin_addr.s_addr;
    } else {
        std::memcpy(result->m_addr6.sin6_addr.s6_addr,
                    reinterpret_cast<sockaddr_in6*>(temp->ai_addr)->sin6_addr.s6_addr,
                    sizeof(result->m_addr6.sin6_addr.s6_addr));
    }
    result->m_addr.sa_family = temp->ai_family;
    freeaddrinfo(res);
    return true;
}

} // namespace flute