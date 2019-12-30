//
// Created by why on 2019/12/30.
//

#include <flute/Acceptor.h>
#include <flute/Channel.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Socket.h>
#include <flute/socket_ops.h>

#include <cstring>

namespace flute {

Acceptor::Acceptor(EventLoop* loop)
    : m_listening(false)
    , m_reuseAddress(true)
    , m_reusePort(true)
    , m_socket(nullptr)
    , m_loop(loop)
    , m_channel(nullptr)
    , m_acceptCallback() {
}

Acceptor::~Acceptor() {
    m_channel->disableAll();
    delete m_channel;
    delete m_socket;
}

void Acceptor::bind(const InetAddress& address) {
    m_socket = new Socket(flute::createNonblockingSocket(address.family()));
    m_socket->bind(address);
    m_socket->setReuseAddress(m_reuseAddress);
    m_socket->setReusePort(m_reusePort);
    m_channel = new Channel(m_socket->descriptor(), m_loop);
    m_channel->setReadCallback(std::bind(&Acceptor::handleRead, this));
}

void Acceptor::setReusePort(bool reusePort) {
    if (m_socket) {
        m_socket->setReusePort(reusePort);
    }
    m_reusePort = reusePort;
}

void Acceptor::setReuseAddress(bool reuseAddress) {
    if (m_socket) {
        m_socket->setReuseAddress(reuseAddress);
    }
    m_reuseAddress = reuseAddress;
}

void Acceptor::listen() {
    m_listening = true;
    m_socket->listen();
    m_channel->enableRead();
}

void Acceptor::close() {
    m_channel->disableAll();
    m_socket->close();
    m_listening = false;
}

void Acceptor::handleRead() {
    auto conn = m_socket->accept();
    if (conn >= 0) {
        if (m_acceptCallback) {
            m_acceptCallback(conn);
        } else {
            flute::closeSocket(conn);
        }
    } else {
        LOG_ERROR << "accept error " << errno << ":" << std::strerror(errno);
    }
}

} // namespace flute