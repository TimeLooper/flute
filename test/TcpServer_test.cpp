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

int main(int argc, char* argv[]) {
    using namespace flute;
    EventLoopGroup boss(1);
    EventLoopGroup child(4);
    TcpServer server(&boss, &child);
    InetAddress address(8000);
    server.bind(address);
    server.sync();
    boss.shutdown();
    child.shutdown();
}