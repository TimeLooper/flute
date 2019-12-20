/*************************************************************************
 *
 * File Name:  EchoServer_test.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/20 00:36:38
 *
 *************************************************************************/

#include <flute/TcpServer.h>
#include <flute/TcpConnection.h>
#include <flute/Logger.h>
#include <flute/Buffer.h>
#include <flute/InetAddress.h>
#include <flute/EventLoopGroup.h>

class EchoServer {
public:
    EchoServer(flute::EventLoopGroup* parent, flute::EventLoopGroup* child) : m_server(parent, child) {
        m_server.setConnectionEstablishedCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
        m_server.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
    }
    
    void bind(const flute::InetAddress& address) {
        m_server.bind(address);
    }

    void sync() {
        m_server.sync();
    }

private:
    void onConnection(const std::shared_ptr<flute::TcpConnection>& conn) {
        LOG_TRACE << conn->getRemoteAddress().toString()<< " -> " << conn->getRemoteAddress().toString() << " is " << (conn->connected() ? "UP" : "DOWN");
        conn->send("hello\n");
    }

    void onMessage(const std::shared_ptr<flute::TcpConnection>& conn, flute::Buffer& buf) {
        // std::string msg(static_cast<const char *>(buf.peek()), buf.readableBytes());
        // buf.retrieveAll();
        buf.setLineSeparator("\n");
        auto msg = buf.readLine();
        if (msg == "exit\n") {
            conn->send("bye");
            conn->shutdown();
        }
        if (msg == "quit\n") {
            m_server.close();
        }
        conn->send(msg);
        // conn->send("shutdown");
    }
    flute::TcpServer m_server;
};

int main(int argc, char* argv[]) {
    LOG_INFO << "sizeof TcpConnection = " << sizeof(flute::TcpConnection);
    std::size_t numThreads = 4;
    if (argc > 1) {
      numThreads = atoi(argv[1]);
    }
    bool ipv6 = argc > 2;
    flute::EventLoopGroup bossGroup(1);
    flute::EventLoopGroup childGroup(numThreads);
    flute::InetAddress listenAddr(2000, false, ipv6);
    EchoServer server(&bossGroup, &childGroup);
    server.bind(listenAddr);
    server.sync();

    // server.start().get();
    bossGroup.shutdown();
    childGroup.shutdown();
    return 0;
}