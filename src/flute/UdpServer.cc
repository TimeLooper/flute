//
// Created by why on 2020/01/06.
//

#include <flute/Channel.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/RingBuffer.h>
#include <flute/Socket.h>
#include <flute/UdpServer.h>
#include <flute/socket_ops.h>

namespace flute {

UdpServer::UdpServer(EventLoopGroup* loopGroup)
    : m_eventLoopGroup(loopGroup), m_channel(nullptr), m_socket(nullptr), m_messageCallback() {}

UdpServer::~UdpServer() {}

void UdpServer::bind(const InetAddress& address) {
    socket_type descriptor = flute::createNonblockingSocket(address.family(), SocketType::DGRAM_SOCKET);
    m_socket = new Socket(descriptor);
    m_channel = new Channel(descriptor, m_eventLoopGroup->getMasterEventLoop());
    m_socket->bind(address);
    m_socket->setReuseAddress(true);
    m_socket->setReusePort(true);
    m_channel->setReadCallback(std::bind(&UdpServer::handleRead, this));
    m_channel->enableRead();
}

flute::ssize_t UdpServer::send(const InetAddress& address, const void* buffer, flute::ssize_t length) {
    msghdr message{};
    iovec vec{};
    message.msg_name = const_cast<sockaddr*>(address.getSocketAddress());
    message.msg_namelen = static_cast<socklen_t>(address.getSocketLength());
    message.msg_iov = &vec;
    message.msg_iovlen = 1;
    vec.iov_base = const_cast<void*>(buffer);
    vec.iov_len = length;
    return flute::sendmsg(m_socket->descriptor(), &message, 0);
}

flute::ssize_t UdpServer::send(const InetAddress& address, const std::string& message) {
    msghdr msg{};
    iovec vec{};
    msg.msg_name = const_cast<sockaddr*>(address.getSocketAddress());
    msg.msg_namelen = static_cast<socklen_t>(address.getSocketLength());
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    vec.iov_base = const_cast<char*>(message.c_str());
    vec.iov_len = message.length();
    return flute::sendmsg(m_socket->descriptor(), &msg, 0);
}

flute::ssize_t UdpServer::send(const InetAddress& address, RingBuffer& buffer) {
    return buffer.sendToSocket(m_socket->descriptor(), address);
}

void UdpServer::close() {
    if (m_channel) {
        m_channel->disableAll();
        delete m_channel;
    }
    if (m_socket) {
        m_socket->close();
        delete m_socket;
    }
}

void UdpServer::handleRead() {
    RingBuffer buffer(1024);
    InetAddress address;
    auto ret = buffer.readFromSocket(m_socket->descriptor(), address);
    if (ret == -1) {
        auto error = flute::getSocketError(m_socket->descriptor());
        LOG_ERROR << "read dgram packet error " << error << ":" << flute::formatErrorString(error);
    } else {
        if (m_messageCallback) {
            m_messageCallback(shared_from_this(), address, buffer);
        }
    }
}

} // namespace flute
