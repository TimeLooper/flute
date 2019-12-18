/*************************************************************************
 *
 * File Name:  TcpServer_test.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/18
 *
 *************************************************************************/

#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/TcpServer.h>
#include <flute/Logger.h>
#include <flute/Buffer.h>
#include <flute/TcpConnection.h>

void handleMessage(const std::shared_ptr<flute::TcpConnection>& conn, flute::Buffer& buffer) {
    LOG_DEBUG << "receive message from " << conn->getRemoteAddress().toString() << " : " << buffer.readLine();
}

int main(int argc, char* argv[]) {
    using namespace flute;
    EventLoopGroup boss(1);
    EventLoopGroup child(4);
    TcpServer server(&boss, &child);
    server.setMessageCallback(handleMessage);
    InetAddress address(8000);
    server.bind(address);
    server.sync();
    boss.shutdown();
    child.shutdown();
}