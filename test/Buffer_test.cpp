/*************************************************************************
 *
 * File Name:  Buffer_test.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/13 00:39:02
 *
 *************************************************************************/

#include <flute/Buffer.h>

int main(int argc, char* argv[]) {
    flute::Buffer buf;
    buf.appendInt32(4);
    buf.appendInt32(8);
    auto ret = buf.readInt32();
    ret = buf.readInt32();
    return 0;
}