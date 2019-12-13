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
    char str[] = "Hello World\r\n";
    buf.appendInt32(4);
    auto ret = buf.readInt32();
    buf.appendInt32(8);
    buf.appendInt32(16);
    buf.appendInt32(32);
    buf.append(reinterpret_cast<std::uint8_t *>(str), sizeof(str) - 1);
    ret = buf.readInt32();
    ret = buf.readInt32();
    ret = buf.readInt32();
    auto s = buf.readLine();
    buf.appendInt32(8);
    buf.appendInt32(16);
    buf.appendInt32(32);
    ret = buf.readInt32();
    ret = buf.readInt32();
    ret = buf.readInt32();
    return 0;
}