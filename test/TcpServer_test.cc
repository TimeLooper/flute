//
// Created by why on 2019/12/31.
//

#include <flute/CircularBuffer.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/TcpConnection.h>
#include <flute/TcpServer.h>

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroup loopGroup(0);
    flute::TcpServer server(&loopGroup);
    server.bind(flute::InetAddress(9999));
    server.setMessageCallback(
        [&](const std::shared_ptr<flute::TcpConnection>& conn, flute::CircularBuffer& buffer) {
            char temp[4096] = { 0 };
            auto length = buffer.read(temp, static_cast<flute::ssize_t>(sizeof(temp)));
            conn->send(temp, length);
    });
    loopGroup.dispatch();
    flute::deinitialize();
    return 0;
}