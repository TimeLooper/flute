//
// Created by why on 2020/01/06.
//

#include <flute/RingBuffer.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/UdpServer.h>

void handleMessage(const std::shared_ptr<flute::UdpServer>& server, const flute::InetAddress& address,
                   flute::RingBuffer& buffer) {
    char temp[4096] = {0};
    auto length = buffer.read(temp, static_cast<flute::ssize_t>(sizeof(temp)));
    LOG_DEBUG << "receive message from " << address.toString() << " -> " << temp;
    server->send(address, temp, length);
}

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroupConfigure configure(false, 0, 0);
    flute::EventLoopGroup group(configure);
    std::shared_ptr<flute::UdpServer> server(new flute::UdpServer(&group));
    flute::InetAddress address(9999);
    server->bind(address);
    server->setMessageCallback(handleMessage);
    group.dispatch();
    flute::deinitialize();
    return 0;
}