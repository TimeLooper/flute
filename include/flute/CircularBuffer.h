//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_CIRCULAR_BUFFER_H
#define FLUTE_CIRCULAR_BUFFER_H

#include <flute/config.h>
#include <flute/copyable.h>
#include <flute/flute_types.h>

#include <string>

namespace flute {

class InetAddress;

class CircularBuffer : private copyable {
public:
    FLUTE_API_DECL CircularBuffer();
    FLUTE_API_DECL CircularBuffer(const CircularBuffer& buffer);
    FLUTE_API_DECL CircularBuffer(CircularBuffer&& buffer) noexcept;
    FLUTE_API_DECL explicit CircularBuffer(flute::ssize_t size);
    FLUTE_API_DECL ~CircularBuffer();

    FLUTE_API_DECL CircularBuffer& operator=(const CircularBuffer& buffer);
    FLUTE_API_DECL CircularBuffer& operator=(CircularBuffer&& buffer) noexcept;
    FLUTE_API_DECL void swap(CircularBuffer& buffer);

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
    FLUTE_API_DECL void append(CircularBuffer& buffer);
    FLUTE_API_DECL void append(CircularBuffer& buffer, flute::ssize_t length);
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
    FLUTE_API_DECL flute::ssize_t readFromSocket(socket_type descriptor, InetAddress& address);
    FLUTE_API_DECL flute::ssize_t sendToSocket(socket_type descriptor, const InetAddress& address);
    FLUTE_API_DECL void clear();

private:
    flute::ssize_t m_readIndex;
    flute::ssize_t m_writeIndex;
    flute::ssize_t m_bufferSize;
    flute::ssize_t m_capacity;
    std::uint8_t* m_buffer;
    std::string m_lineSeparator;

    void expand(flute::ssize_t length);
    void appendInternal(const CircularBuffer& buffer, flute::ssize_t length);
};

} // namespace flute

#endif // FLUTE_CIRCULAR_BUFFER_H