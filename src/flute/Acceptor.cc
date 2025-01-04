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
    , m_family(0)
    , m_socket(nullptr)
    , m_loop(loop)
    , m_channel(nullptr)
    , m_acceptCallback()
    , m_ioContext()
{
}

Acceptor::~Acceptor() {
    assert(!m_socket);
    assert(!m_channel);
    auto asyncIoService = m_loop->getAsyncIoService();
    if (asyncIoService) {
        for (auto i = 0; i < kIoContextSize; ++i) {
            if (m_ioContext[i]) {
                asyncIoService->destroyIoContext(m_ioContext[i]);
            }
        }
    }
    if (m_buffer) {
        delete[] m_buffer;
    }
}

void Acceptor::bind(const InetAddress& address) {
    m_socket = new Socket(flute::createNonblockingSocket(address.family(), SocketType::STREAM_SOCKET));
    m_socket->bind(address);
    m_socket->setReuseAddress(m_reuseAddress);
    m_socket->setReusePort(m_reusePort);
    m_channel = new Channel(m_socket->descriptor(), m_loop, m_loop->getAsyncIoService() != nullptr);
    m_channel->setReadCallback(std::bind(&Acceptor::handleRead, this));
    m_family = address.family();
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
    auto asyncIoService = m_loop->getAsyncIoService();
    if (asyncIoService) {
        for (auto i = 0; i < kIoContextSize; ++i) {
            auto socket = flute::createNonblockingSocket(m_family, SocketType::STREAM_SOCKET);
            auto addrlen = 0;
            if (m_family == AF_INET) {
                addrlen = sizeof(sockaddr_in);
            } else if (m_family == AF_INET6) {
                addrlen = sizeof(sockaddr_in6);
            } else {
                return;
            }
            auto bufflen = (addrlen + 16) * 2;
            char* buffer = new char[(addrlen + 16) * 2];
            m_buffer = buffer;
            auto context = asyncIoService->createIoContext();
            context->socket = m_socket->descriptor();
            context->acceptSocket = socket;
            context->opCode = SocketOpCode::Accept;
            iovec vec;
            vec.iov_base = buffer;
            vec.iov_len = bufflen;
            asyncIoService->setIoContextBuffer(context, &vec, 1);
            context->ioCompleteCallback = std::bind(&Acceptor::handleAsyncAccept, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
            m_ioContext[i] = context;
            asyncIoService->bindIoService(m_socket->descriptor());
            asyncIoService->post(context);
        }
    }
}

void Acceptor::close() {
    m_channel->disableAll();
    m_socket->close();
    delete m_channel;
    delete m_socket;
    m_channel = nullptr;
    m_socket = nullptr;
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
        auto error = getLastError();
        LOG_ERROR << "accept error " << error << ":" << formatErrorString(error);
    }
}

void Acceptor::handleAsyncAccept(AsyncIoCode code, ssize_t bytes, AsyncIoContext* ioContext) {
    auto asyncIoService = m_loop->getAsyncIoService();
    if (asyncIoService) {
        if (code == AsyncIoCode::IoCodeSuccess) {
            auto socket = ioContext->acceptSocket;
            if (m_loop->isInLoopThread()) {
                handleAsyncAcceptInLoop(socket);
            } else {
                m_loop->queueInLoop(std::bind(&Acceptor::handleAsyncAcceptInLoop, this, socket));
            }
            ioContext->acceptSocket = flute::createNonblockingSocket(m_family, SocketType::STREAM_SOCKET);
            asyncIoService->post(ioContext);
        } else {
            auto error = getLastError();
            LOG_ERROR << "accept error " << error << ":" << formatErrorString(error);
        }
    }
}

void Acceptor::handleAsyncAcceptInLoop(socket_type socket) {
    if (m_acceptCallback) {
        m_acceptCallback(socket);
    } else {
        flute::closeSocket(socket);
    }
}

} // namespace flute