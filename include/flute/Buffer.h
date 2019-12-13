/*************************************************************************
 *
 * File Name:  Buffer.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/08 21:30:50
 *
 *************************************************************************/

#pragma once

#include <flute/config.h>

#include <list>
#include <string>

namespace flute {

class Buffer {
public:
    FLUTE_API_DECL Buffer();
    FLUTE_API_DECL ~Buffer();

    FLUTE_API_DECL std::size_t readableBytes() const;
    FLUTE_API_DECL std::size_t writeableBytes() const;
    FLUTE_API_DECL std::int8_t peekInt8() const;
    FLUTE_API_DECL std::int16_t peekInt16() const;
    FLUTE_API_DECL std::int32_t peekInt32() const;
    FLUTE_API_DECL std::int64_t peekInt64() const;
    FLUTE_API_DECL std::string peekLine() const;
    FLUTE_API_DECL void peek(std::uint8_t* buffer, std::size_t length) const;
    FLUTE_API_DECL std::int8_t readInt8();
    FLUTE_API_DECL std::int16_t readInt16();
    FLUTE_API_DECL std::int32_t readInt32();
    FLUTE_API_DECL std::int64_t readInt64();
    FLUTE_API_DECL std::string readLine();
    FLUTE_API_DECL void read(std::uint8_t* buffer, std::size_t length);
    FLUTE_API_DECL void append(const std::uint8_t* buffer, std::size_t length);
    FLUTE_API_DECL void appendInt8(std::int8_t value);
    FLUTE_API_DECL void appendInt16(std::int16_t value);
    FLUTE_API_DECL void appendInt32(std::int32_t value);
    FLUTE_API_DECL void appendInt64(std::int64_t value);
    FLUTE_API_DECL void setLineSeparator(std::string&& separator);
    FLUTE_API_DECL void setLineSeparator(const std::string& separator);
    FLUTE_API_DECL const std::string& getLineSeparator() const;

private:
    std::size_t m_readIndex;
    std::size_t m_writeIndex;
    std::size_t m_bufferSize;
    std::size_t m_capacity;
    std::uint8_t* m_buffer;
    std::string m_lineSeparator;

    std::size_t getCapacity(std::size_t length);
};

} // namespace flute