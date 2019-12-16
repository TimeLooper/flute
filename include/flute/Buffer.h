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
#include <flute/copyable.h>
#include <flute/flute_types.h>

#include <list>
#include <string>

namespace flute {

class Buffer : private copyable {
public:
    FLUTE_API_DECL Buffer();
    FLUTE_API_DECL Buffer(const Buffer& buffer);
    FLUTE_API_DECL Buffer(Buffer&& buffer);
    FLUTE_API_DECL ~Buffer();

    FLUTE_API_DECL Buffer& operator=(const Buffer& buffer);
    FLUTE_API_DECL Buffer& operator=(Buffer&& buffer);
    FLUTE_API_DECL void swap(Buffer& buffer);

    FLUTE_API_DECL flute::ssize_t readableBytes() const;
    FLUTE_API_DECL flute::ssize_t writeableBytes() const;
    FLUTE_API_DECL std::int8_t peekInt8() const;
    FLUTE_API_DECL std::int16_t peekInt16() const;
    FLUTE_API_DECL std::int32_t peekInt32() const;
    FLUTE_API_DECL std::int64_t peekInt64() const;
    FLUTE_API_DECL std::string peekLine() const;
    FLUTE_API_DECL void peek(void* buffer, flute::ssize_t length) const;
    FLUTE_API_DECL std::int8_t readInt8();
    FLUTE_API_DECL std::int16_t readInt16();
    FLUTE_API_DECL std::int32_t readInt32();
    FLUTE_API_DECL std::int64_t readInt64();
    FLUTE_API_DECL std::string readLine();
    FLUTE_API_DECL void read(void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void append(Buffer& buffer);
    FLUTE_API_DECL void append(const void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void appendInt8(std::int8_t value);
    FLUTE_API_DECL void appendInt16(std::int16_t value);
    FLUTE_API_DECL void appendInt32(std::int32_t value);
    FLUTE_API_DECL void appendInt64(std::int64_t value);
    FLUTE_API_DECL void setLineSeparator(std::string&& separator);
    FLUTE_API_DECL void setLineSeparator(const std::string& separator);
    FLUTE_API_DECL const std::string& getLineSeparator() const;
    FLUTE_API_DECL flute::ssize_t readFromSocket(socket_type descriptor);
    FLUTE_API_DECL flute::ssize_t sendToSocket(socket_type descriptor);

private:
    flute::ssize_t m_readIndex;
    flute::ssize_t m_writeIndex;
    flute::ssize_t m_bufferSize;
    flute::ssize_t m_capacity;
    std::uint8_t* m_buffer;
    std::string m_lineSeparator;

    void expand(flute::ssize_t length);
    void appendInternal(const Buffer& buffer);
};

} // namespace flute