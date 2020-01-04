//
// Created by why on 2019/12/31.
//

#include <flute/TcpServer.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Buffer.h>
#include <flute/TcpConnection.h>

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroup loopGroup(0);
    flute::TcpServer server(&loopGroup);
    server.bind(flute::InetAddress(9999));
    server.setMessageCallback([&](const std::shared_ptr<flute::TcpConnection>& conn, flute::Buffer& buffer) {
        conn->send(buffer);
    });
    loopGroup.dispatch();
    flute::deinitialize();
    return 0;
}