/*************************************************************************
 *
 * File Name:  Buffer.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/08 21:30:50
 *
 *************************************************************************/

#pragma once

#include <string>
#include <list>

namespace flute {

struct BufferChain {
    std::size_t readIndex;
    std::size_t writeIndex;
    std::size_t capacity;
    std::uint8_t* buffer;
};

class Buffer {
public:
    Buffer();
    ~Buffer();

    std::int8_t peekInt8() const;
    std::int16_t peekInt16() const;
    std::int32_t peekInt32() const;
    std::int64_t peekInt64() const;
    std::int8_t readInt8();
    std::int16_t readInt16();
    std::int32_t readInt32();
    std::int64_t readInt64();
    std::string readLine();
    void appendInt8(std::int8_t value);
    void appendInt16(std::int16_t value);
    void appendInt32(std::int32_t value);
    void appendInt64(std::int64_t value);
    void appendBufferChain(BufferChain* ch);
    void append(const void* buffer, std::size_t size);

    static BufferChain* createBufferChain(std::size_t length = 4096);
    static void destroyBufferChain(BufferChain** chain);

private:
    void peek(std::uint8_t* buffer, std::size_t length);
    std::list<BufferChain *> m_chains;
};

} // namespace flute