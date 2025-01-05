//
// Created by why on 2020/01/05.
//

#include <flute/RingBuffer.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/TcpClient.h>
#include <flute/TcpConnection.h>
#include <flute/flute_types.h>

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroupConfigure configure(true, 0, 1);
    flute::EventLoopGroup group(configure);
    flute::TcpClient client(&group, flute::InetAddress("127.0.0.1", 9999));
    client.setMessageCallback([&](const std::shared_ptr<flute::TcpConnection>& conn, flute::RingBuffer& buffer) {
        char temp[4096] = { 0 };
        auto length = buffer.read(temp, static_cast<flute::ssize_t>(sizeof(temp)));
        LOG_DEBUG << "receive message from " << conn->getRemoteAddress().toString() << ":" << temp;
        conn->send(temp, length);
    });
    client.setConnectionEstablishedCallback([&](const std::shared_ptr<flute::TcpConnection>& conn) {
        conn->send("hello\n");
    });
    client.connect();
    group.dispatch();
    flute::deinitialize();
    return 0;
}