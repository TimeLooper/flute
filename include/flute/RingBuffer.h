//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_CIRCULAR_BUFFER_H
#define FLUTE_CIRCULAR_BUFFER_H

#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <cstddef>
#include <string>
#include <vector>

namespace flute {

class InetAddress;
class ByteBuffer;

class RingBuffer : private noncopyable {
public:
    FLUTE_API_DECL RingBuffer(flute::ssize_t size);
    FLUTE_API_DECL RingBuffer(RingBuffer&& buffer) noexcept;
    FLUTE_API_DECL ~RingBuffer();

    FLUTE_API_DECL RingBuffer& operator=(RingBuffer&& buffer) noexcept;
    FLUTE_API_DECL void swap(RingBuffer& buffer);

    FLUTE_API_DECL flute::ssize_t readableBytes() const;
    FLUTE_API_DECL flute::ssize_t writeableBytes() const;
    FLUTE_API_DECL std::int8_t peekInt8() const;
    FLUTE_API_DECL std::int16_t peekInt16() const;
    FLUTE_API_DECL std::int32_t peekInt32() const;
    FLUTE_API_DECL std::int64_t peekInt64() const;
    FLUTE_API_DECL float peekFloat() const;
    FLUTE_API_DECL double peekDouble() const;
    FLUTE_API_DECL flute::ssize_t peek(void* buffer, flute::ssize_t length) const;
    FLUTE_API_DECL std::int8_t readInt8();
    FLUTE_API_DECL std::int16_t readInt16();
    FLUTE_API_DECL std::int32_t readInt32();
    FLUTE_API_DECL std::int64_t readInt64();
    FLUTE_API_DECL float readFloat();
    FLUTE_API_DECL double readDouble();
    FLUTE_API_DECL flute::ssize_t read(void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void read(ByteBuffer& buffer, flute::ssize_t length);
    FLUTE_API_DECL void append(RingBuffer& buffer);
    FLUTE_API_DECL void append(RingBuffer& buffer, flute::ssize_t length);
    FLUTE_API_DECL void append(const void* buffer, flute::ssize_t length);
    FLUTE_API_DECL void append(ByteBuffer& buffer);
    FLUTE_API_DECL void appendInt8(std::int8_t value);
    FLUTE_API_DECL void appendInt16(std::int16_t value);
    FLUTE_API_DECL void appendInt32(std::int32_t value);
    FLUTE_API_DECL void appendInt64(std::int64_t value);
    FLUTE_API_DECL void appendFloat(float value);
    FLUTE_API_DECL void appendDouble(double value);
    FLUTE_API_DECL flute::ssize_t readFromSocket(socket_type descriptor);
    FLUTE_API_DECL flute::ssize_t sendToSocket(socket_type descriptor);
    FLUTE_API_DECL flute::ssize_t readFromSocket(socket_type descriptor, InetAddress& address);
    FLUTE_API_DECL flute::ssize_t sendToSocket(socket_type descriptor, const InetAddress& address);
    FLUTE_API_DECL void clear();
    FLUTE_API_DECL void getReadableBuffer(std::vector<iovec>& vecs);
    FLUTE_API_DECL void getWriteableBuffer(std::vector<iovec>& vecs);
    FLUTE_API_DECL void updateWriteIndex(flute::ssize_t length);

private:
    flute::ssize_t m_readIndex;
    flute::ssize_t m_writeIndex;
    flute::ssize_t m_bufferSize;
    flute::ssize_t m_capacity;
    std::uint8_t* m_buffer;

    void expand(flute::ssize_t length);
    void appendInternal(const RingBuffer& buffer, flute::ssize_t length);
};

} // namespace flute

#endif // FLUTE_CIRCULAR_BUFFER_H
