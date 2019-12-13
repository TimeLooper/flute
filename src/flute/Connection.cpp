/*************************************************************************
 *
 * File Name:  Connection.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/05 23:46:29
 *
 *************************************************************************/

#include <flute/Connection.h>
#include <flute/socket_ops.h>

namespace flute {

Connection::Connection(socket_type descriptor, EventLoop* loop, const sockaddr_storage& localAddress,
                       const sockaddr_storage& remoteAddress)
    : m_descriptor(descriptor), m_highWaterMark(0), m_loop(loop), m_state(ConnectionState::DISCONNECTED), m_localAddress(localAddress), m_remoteAddress(remoteAddress) {
}

Connection::~Connection() {
}

void Connection::setMessageCallback(const MessageCallback& cb) {
    m_messageCallback = cb;
}

void Connection::setMessageCallback(MessageCallback&& cb) {
    m_messageCallback = std::move(cb);
}

void Connection::setCloseCallback(const CloseCallback& cb) {
    m_closeCallback = cb;
}

void Connection::setCloseCallback(CloseCallback&& cb) {
    m_closeCallback = std::move(cb);
}

void Connection::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    m_writeCompleteCallback = cb;
}

void Connection::setWriteCompleteCallback(WriteCompleteCallback&& cb) {
    m_writeCompleteCallback = std::move(cb);
}

void Connection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb) {
    m_highWaterMarkCallback = cb;
}

void Connection::setHighWaterMarkCallback(HighWaterMarkCallback&& cb) {
    m_highWaterMarkCallback = std::move(cb);
}

socket_type Connection::descriptor() const {
    return m_descriptor;
}

void Connection::setTcpNoDelay(bool on) const {
    int option = on ? 1 : 0;
    flute::setsockopt(m_descriptor, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&option), sizeof(option));
}

EventLoop* Connection::getEventLoop() const {
    return m_loop;
}

const sockaddr_storage& Connection::getLocalAddress() const {
    return m_localAddress;
}

const sockaddr_storage& Connection::getRemoteAddress() const {
    return m_remoteAddress;
}

} // namespace flute