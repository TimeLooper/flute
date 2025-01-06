//
// Created by why on 2019/12/31.
//

#include <flute/RingBuffer.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/TcpConnection.h>
#include <flute/TcpServer.h>
#include <flute/Logger.h>

#include <csignal>

static std::function<void()> h;
void handler(int sig) {
    if (sig == SIGINT) {
        h();
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, handler);
    flute::initialize();
    flute::EventLoopGroupConfigure configure(true, 4, 4);
    flute::EventLoopGroup loopGroup(configure);
    flute::TcpServer server(&loopGroup);
    server.bind(flute::InetAddress(9999));
    server.setMessageCallback(
        [&](const std::shared_ptr<flute::TcpConnection>& conn, flute::RingBuffer& buffer) {
            char temp[4096] = { 0 };
            auto length = buffer.read(temp, static_cast<flute::ssize_t>(sizeof(temp)));
            conn->send(temp, length);
    });
    h = [&] {
        server.close();
        loopGroup.shutdown();
    };
    loopGroup.wait();
    flute::deinitialize();
    return 0;
}