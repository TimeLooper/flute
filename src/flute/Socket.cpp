/*************************************************************************
 *
 * File Name:  Socket.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/17
 *
 *************************************************************************/

#include <flute/Logger.h>
#include <flute/Socket.h>

#include <cerrno>
#include <cstring>

namespace flute {

Socket::Socket(socket_type descriptor) : m_descriptor(descriptor) {}

Socket::~Socket() { close(); }

void Socket::bind(const InetAddress &address) {
    auto result = flute::bind(m_descriptor, address);
    if (result != 0) {
        LOG_ERROR << "Socket::bind(" << m_descriptor << ") error " << errno << ":" << std::strerror(errno);
    }
}

void Socket::listen() {
    auto result = flute::listen(m_descriptor);
    if (result != 0) {
        LOG_ERROR << "Socket::listen(" << m_descriptor << ") error " << errno << ":" << std::strerror(errno);
    }
}

socket_type Socket::accept() {
    return flute::accept(m_descriptor, nullptr);
}

void Socket::setTcpNoDelay(bool on) {
    int option = on ? 1 : 0;
    flute::setsockopt(m_descriptor, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&option), sizeof(option));
}

void Socket::setReuseAddress(bool on) {
    int option = on ? 1 : 0;
    flute::setsockopt(m_descriptor, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&option), sizeof(option));
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
    int option = on ? 1 : 0;
    auto result = flute::setsockopt(m_descriptor, SOL_SOCKET, SO_REUSEPORT, &option, sizeof(option));
    if (result < 0 && on) {
        LOG_ERROR << "set socket reuse port failed.";
    }
#else
    if (on) {
        LOG_ERROR << "socket reuse port is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on) {
    int option = on ? 1 : 0;
    flute::setsockopt(m_descriptor, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&option), sizeof(option));
}

void Socket::shutdownWrite() {
    if (flute::shutdown(m_descriptor, SHUT_WR) < 0) {
        LOG_ERROR << "shutdown socket " << m_descriptor << " write with error " << errno << ":" << std::strerror(errno);
    }
}

void Socket::shutdownRead() {
    if (flute::shutdown(m_descriptor, SHUT_RD) < 0) {
        LOG_ERROR << "shutdown socket " << m_descriptor << " read with error " << errno << ":" << std::strerror(errno);
    }
}

void Socket::close() { flute::closeSocket(m_descriptor); }

} // namespace flute