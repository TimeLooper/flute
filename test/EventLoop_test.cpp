/*************************************************************************
 *
 * File Name:  EventLoop_test.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/01 01:02:33
 *
 *************************************************************************/

#include <flute/Logger.h>
#include <flute/EventLoop.h>

int main(int argc, char* argv[]) {
    flute::EventLoop loop;
    loop.attachThread();
    auto delay = 1000;
    loop.schedule([=] {
        LOG_DEBUG << "timer" << delay;
    }, delay, -1);
    delay = 500;
    loop.schedule([=] {
        LOG_DEBUG << "timer" << delay;
    }, delay, 10);
    delay = 500;
    loop.schedule([=] {
        LOG_DEBUG << "timer" << delay;
    }, delay, 2);
    loop.dispatch();
    return 0;
}