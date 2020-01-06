//
// Created by why on 2020/01/06.
//

#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/UdpServer.h>
#include <flute/Buffer.h>

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroup group(0);
    flute::UdpServer server(&group);
    flute::InetAddress address(9999);
    server.bind(address);
    server.setMessageCallback([&](flute::InetAddress address, flute::Buffer& buffer) {
        buffer.setLineSeparator("\n");
        LOG_DEBUG << "receive message from " << address.toString() << " -> " << buffer.readLine();
    });
    group.dispatch();
    flute::deinitialize();
    return 0;
}