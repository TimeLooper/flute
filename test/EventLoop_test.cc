//
// Created by why on 2019/12/31.
//

#include <flute/EventLoop.h>
#include <flute/Logger.h>
#include <flute/socket_ops.h>

int main(int argc, char* argv[]) {
    flute::initialize();
    flute::EventLoop loop;
    loop.schedule([&] {
        LOG_DEBUG << "schedule 1000.";
    }, 1000, -1);
    loop.schedule([&] {
        LOG_DEBUG << "schedule 500.";
    }, 500, -1);
    loop.schedule([&] {
        LOG_DEBUG << "schedule 2000.";
    }, 2000, -1);
    loop.dispatch();
    flute::deinitialize();
}