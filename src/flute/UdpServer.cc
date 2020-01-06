//
// Created by why on 2020/01/06.
//

#include <flute/UdpServer.h>
#include <flute/socket_ops.h>
#include <flute/InetAddress.h>
#include <flute/EventLoopGroup.h>
#include <flute/Channel.h>
#include <flute/Socket.h>
#include <flute/Buffer.h>
#include <flute/Logger.h>

namespace flute {

UdpServer::UdpServer(EventLoopGroup* loopGroup) : m_eventLoopGroup(loopGroup), m_channel(nullptr), m_socket(nullptr), m_connections(), m_messageCallback() {

}

UdpServer::~UdpServer() {

}

void UdpServer::bind(const InetAddress& address) {
    socket_type descriptor = flute::createNonblockingSocket(address.family(), SocketType::DGRAM_SOCKET);
    m_socket = new Socket(descriptor);
    m_channel = new Channel(descriptor, m_eventLoopGroup->getMasterEventLoop());
    m_socket->bind(address);
    m_socket->setReuseAddress(true);
    m_socket->setReusePort(true);
    m_channel->setReadCallback(std::bind(&UdpServer::handleRead, this));
    m_channel->enableRead();
    for (auto i = 0; i < m_eventLoopGroup->getChildLoopSize(); ++i) {
        socket_type child = dup(m_socket->descriptor());
    }
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
    Buffer buffer;
    InetAddress address;
    auto ret = buffer.readFromSocket(m_socket->descriptor(), address);
    if (ret == -1) {
        auto error = flute::getSocketError(m_socket->descriptor());
        LOG_ERROR << "read dgram packet error " << error << ":" << flute::formatErrorString(error);
    } else {
        if (m_messageCallback) {
            m_messageCallback(address, buffer);
        }
    }
}

void UdpServer::handleWrite() {

}

} // namespace flute
