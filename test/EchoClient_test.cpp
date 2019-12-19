/*************************************************************************
 *
 * File Name:  EchoClient_test.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/20 00:52:43
 *
 *************************************************************************/

#include <flute/EventLoopGroup.h>
#include <flute/TcpClient.h>
#include <flute/Logger.h>
#include <flute/noncopyable.h>
#include <flute/TcpConnection.h>
#include <flute/Buffer.h>

class EchoClient : flute::noncopyable {
public:
    EchoClient(flute::EventLoopGroup* loop, const flute::InetAddress& listenAddr)
    : m_client(loop, listenAddr) {
        m_client.setConnectionEstablishedCallback(std::bind(&EchoClient::onConnection, this, std::placeholders::_1));
        m_client.setMessageCallback(std::bind(&EchoClient::onMessage, this, std::placeholders::_1, std::placeholders::_2));
        //client_.enableRetry();
    }

    void connect() {
        m_client.connect();
    }

    void sync() {
        m_client.sync();
    }

    // void stop();

private:
    void onConnection(const std::shared_ptr<flute::TcpConnection>& conn) {
        LOG_TRACE << conn->getLocalAddress().toString() << " -> "
            << conn->getRemoteAddress().toString() << " is "
            << (conn->connected() ? "UP" : "DOWN");

        // if (conn->connected()) {
        //   ++current;
        //   if (current < static_cast<int>(clients.size())) {
        //       clients[current]->connect();
        //   } else {
        //       LOG_INFO << "connect success";
        //   }
        //   LOG_INFO << "*** connected " << current;
        // }
        conn->send("world");
    }

    void onMessage(const std::shared_ptr<flute::TcpConnection>& conn, flute::Buffer& buf) {
        // std::string msg(static_cast<const char *>(buf.peek()), buf.readableBytes());
        // buf.retrieveAll();
        buf.setLineSeparator("\n");
        auto msg = buf.readLine();
        LOG_TRACE << conn->descriptor() << " recv " << msg.size() << " bytes : " << msg;
        if (msg == "quit\n") {
            conn->send("bye");
            conn->shutdown();
        } else if (msg == "shutdown") {
            this->m_client.disconnect();
        } else {
            conn->send(msg);
        }
    }

    flute::TcpClient m_client;
};

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::vector<std::unique_ptr<EchoClient>> clients;
        // std::vector<std::future<void>> futures;
        flute::EventLoopGroup loop(1);
        bool ipv6 = argc > 3;
        flute::InetAddress serverAddr(argv[1], 2000, ipv6);

        int n = 1;
        if (argc > 2) {
            n = atoi(argv[2]);
        }

        clients.reserve(n);
        for (int i = 0; i < n; ++i) {
            clients.emplace_back(new EchoClient(&loop, serverAddr));
            clients[i]->connect();
        }
        for (int i = 0; i < n; ++i) {
            // clients.emplace_back(new EchoClient(&loop, serverAddr));
            clients[i]->sync();
        }
    } else {
        printf("Usage: %s host_ip [current#]\n", argv[0]);
    }
    return 0;
}