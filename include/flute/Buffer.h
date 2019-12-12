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

enum StringEOLStyle {
    STRING_EOL_ANY,
    STRING_EOL_CRLF,
    STRING_EOL_CRLF_STRICT,
    STRING_EOL_LF,
    STRING_EOL_NULL
};

class Buffer {
public:
    Buffer();
    ~Buffer();

    std::size_t readableBytes() const;
    std::int8_t peekInt8() const;
    std::int16_t peekInt16() const;
    std::int32_t peekInt32() const;
    std::int64_t peekInt64() const;
    std::int8_t readInt8();
    std::int16_t readInt16();
    std::int32_t readInt32();
    std::int64_t readInt64();
    std::string readLine();
    void append(const std::uint8_t* buffer, std::size_t length);
    void appendInt8(std::int8_t value);
    void appendInt16(std::int16_t value);
    void appendInt32(std::int32_t value);
    void appendInt64(std::int64_t value);

private:
    std::size_t m_readIndex;
    std::size_t m_writeIndex;
    std::size_t m_capacity;
    std::uint8_t* m_buffer;
};

} // namespace flute