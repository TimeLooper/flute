//
// Created by why on 2020/01/06.
//

#include <flute/Buffer.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/TcpServer.h>
#include <flute/socket_ops.h>

static void receiveMessage(const flute::TcpConnectionPtr& conn, flute::Buffer& buffer) {
    auto length = buffer.readInt32();
    if (length < buffer.readableBytes() - sizeof(length)) {
        // half packet
        return;
    }
    flute::Buffer output;
    output.appendInt32(length);
    output.append(buffer, length);
}

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroup group(3);
    flute::TcpServer server(&group);
    flute::InetAddress address(7681);
    server.bind(address);
    server.setMessageCallback(receiveMessage);
    group.dispatch();
    flute::deinitialize();
    return 0;
}