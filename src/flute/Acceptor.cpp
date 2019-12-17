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
#include <flute/socket_ops.h>
#include <flute/Socket.h>

namespace flute {

Acceptor::Acceptor(flute::EventLoop* loop, const sockaddr_storage& address, bool reusePort)
    : m_listening(false)
    , m_idleDescriptor(createNonblockingSocket(AF_INET))
    , m_loop(loop)
    , m_socket(new Socket(createNonblockingSocket(address.ss_family)))
    , m_channel(new Channel(m_socket->descriptor(), loop))
    , m_acceptCallback() {
    m_socket->setReuseAddress(true);
    m_socket->setReusePort(reusePort);
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
        flute::listen(this->m_channel->descriptor());
        this->m_channel->enableRead();
    });
}

void Acceptor::close() {
    m_loop->runInLoop([this] {
        this->m_loop->assertInLoopThread();
        this->m_channel->disableAll();
        flute::closeSocket(this->m_channel->descriptor());
        flute::closeSocket(this->m_idleDescriptor);
        this->m_listening = false;
    });
}

void Acceptor::handleRead() {
    sockaddr_storage address{};
    auto conn = flute::accept(m_channel->descriptor(), address);
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
            conn = flute::accept(m_channel->descriptor(), address);
            flute::closeSocket(conn);
            m_idleDescriptor = flute::createNonblockingSocket(AF_INET);
        }
    }
}

} // namespace flute
