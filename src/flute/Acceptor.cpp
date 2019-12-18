/*************************************************************************
 *
 * File Name:  Acceptor.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/16
 *
 *************************************************************************/

#include <flute/Acceptor.h>
#include <flute/Channel.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Socket.h>
#include <flute/socket_ops.h>

namespace flute {

Acceptor::Acceptor(EventLoopGroup* loopGroup, const InetAddress& address, bool reusePort)
    : m_listening(false)
    , m_idleDescriptor(createNonblockingSocket(AF_INET))
    , m_socket(new Socket(createNonblockingSocket(address.family())))
    , m_loop(loopGroup->chooseEventLoop(m_socket->descriptor()))
    , m_channel(new Channel(m_socket->descriptor(), m_loop))
    , m_acceptCallback() {
    m_socket->setReuseAddress(true);
    m_socket->setReusePort(reusePort);
    m_socket->bind(address);
    m_channel->setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    m_channel->disableAll();
    delete m_channel;
    delete m_socket;
}

void Acceptor::listen() {
    m_loop->runInLoop([this] {
        this->m_loop->assertInLoopThread();
        this->m_listening = true;
        this->m_socket->listen();
        this->m_channel->enableRead();
        LOG_DEBUG << "listen";
    });
}

void Acceptor::close() {
    m_loop->runInLoop([this] {
        this->m_loop->assertInLoopThread();
        this->m_channel->disableAll();
        this->m_socket->close();
        flute::closeSocket(this->m_idleDescriptor);
        this->m_listening = false;
    });
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
        if (errno == EMFILE) {
            LOG_WARN << "server busy.";
            flute::closeSocket(m_idleDescriptor);
            conn = m_socket->accept();
            flute::closeSocket(conn);
            m_idleDescriptor = flute::createNonblockingSocket(AF_INET);
        }
    }
}

} // namespace flute
