/*************************************************************************
 *
 * File Name:  ThreadPool_test.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/15
 *
 *************************************************************************/

#include <flute/Logger.h>
#include <flute/ThreadPool.h>

int add(int a, int b) {
    return a + b;
}

int main(int argc, char* argv[]) {
    flute::ThreadPool threadPool;
    threadPool.start(4);
    auto result = threadPool.execute(add, 1, 2);
    LOG_DEBUG << "result = " << result.get();
    threadPool.shutdown();
    return 0;
}
