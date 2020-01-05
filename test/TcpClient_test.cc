//
// Created by why on 2020/01/05.
//

#include <flute/EventLoopGroup.h>
#include <flute/TcpClient.h>
#include <flute/TcpConnection.h>
#include <flute/InetAddress.h>
#include <flute/Buffer.h>
#include <flute/flute_types.h>
#include <flute/Logger.h>

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroup group(0);
    flute::TcpClient client(&group, flute::InetAddress("127.0.0.1", 9999));
    client.setMessageCallback([&](const std::shared_ptr<flute::TcpConnection>& conn, flute::Buffer& buffer) {
        buffer.setLineSeparator("\n");
        auto msg = buffer.readLine();
        LOG_DEBUG << "receive message from " << conn->getRemoteAddress().toString() << ":" << msg;
        conn->send(msg);
    });
    client.setConnectionEstablishedCallback([&](const std::shared_ptr<flute::TcpConnection>& conn) {
        conn->send("hello\n");
    });
    client.connect();
    group.dispatch();
    flute::deinitialize();
    return 0;
}