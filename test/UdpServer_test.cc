//
// Created by why on 2020/01/06.
//

#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/UdpServer.h>
#include <flute/Buffer.h>

void handleMessage(const std::shared_ptr<flute::UdpServer>& server, const flute::InetAddress& address, flute::Buffer& buffer) {
    buffer.setLineSeparator("\n");
    std::string message = buffer.readLine();
    LOG_DEBUG << "receive message from " << address.toString() << " -> " << message;
    server->send(address, message);
}

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoopGroup group(0);
    std::shared_ptr<flute::UdpServer> server(new flute::UdpServer(&group));
    flute::InetAddress address(9999);
    server->bind(address);
    server->setMessageCallback(handleMessage);
    group.dispatch();
    flute::deinitialize();
    return 0;
}