//
// Created by why on 2019/12/30.
//

#include <flute/ByteBuffer.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/RingBuffer.h>
#include <flute/endian.h>
#include <flute/socket_ops.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace flute {

#define DETAULT_BUFFER_SIZE 1024

#define UPDATE_READ_INDEX(capacity, readIndex, writeIndex, bufferSize, size) \
    do {                                                                     \
        if (bufferSize <= static_cast<flute::ssize_t>(size)) {               \
            readIndex = writeIndex = bufferSize = 0;                         \
        } else {                                                             \
            readIndex += size;                                               \
            readIndex &= capacity - 1;                                       \
            bufferSize -= size;                                              \
        }                                                                    \
    } while (0)

#define UPDATE_WRITE_INDEX(capacity, writeIndex, bufferSize, size) \
    do {                                                           \
        writeIndex += size;                                        \
        writeIndex &= capacity - 1;                                \
        bufferSize += size;                                        \
    } while (0)

inline flute::ssize_t getCapacity(flute::ssize_t length, flute::ssize_t capacity) {
    int result = capacity ? capacity : 1;
    while (result < length + capacity) {
        result <<= 1;
    }
    return result;
}

RingBuffer::RingBuffer(flute::ssize_t size)
    : m_readIndex(0), m_writeIndex(0), m_bufferSize(0), m_capacity(size), m_buffer(nullptr) {
    if (m_capacity > 0) {
        m_buffer = static_cast<std::uint8_t *>(std::malloc(sizeof(std::uint8_t) * size));
    }
}

RingBuffer::RingBuffer(RingBuffer &&buffer) noexcept : RingBuffer(0) { this->swap(buffer); }

RingBuffer::~RingBuffer() {
    if (m_buffer) {
        std::free(m_buffer);
        m_buffer = nullptr;
        m_readIndex = m_writeIndex = m_bufferSize = 0;
    }
}

RingBuffer &RingBuffer::operator=(RingBuffer &&buffer) noexcept {
    this->swap(buffer);
    return *this;
}

void RingBuffer::swap(RingBuffer &buf) {
    std::swap(m_readIndex, buf.m_readIndex);
    std::swap(m_writeIndex, buf.m_writeIndex);
    std::swap(m_bufferSize, buf.m_bufferSize);
    std::swap(m_capacity, buf.m_capacity);
    std::swap(m_buffer, buf.m_buffer);
}

flute::ssize_t RingBuffer::readableBytes() const { return m_bufferSize; }

flute::ssize_t RingBuffer::writeableBytes() const { return m_capacity - m_bufferSize; }

std::int8_t RingBuffer::peekInt8() const {
    std::int8_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return result;
}

std::int16_t RingBuffer::peekInt16() const {
    std::int16_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return flute::network2Host(result);
}

std::int32_t RingBuffer::peekInt32() const {
    std::int32_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return flute::network2Host(result);
}

std::int64_t RingBuffer::peekInt64() const {
    std::int64_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return flute::network2Host(result);
}

float RingBuffer::peekFloat() const {
    float result = 0.0f;
    auto p = reinterpret_cast<std::uint32_t *>(&result);
    peek(p, sizeof(result));
    *p = network2Host(*p);
    return result;
}

double RingBuffer::peekDouble() const {
    double result = 0.0f;
    auto p = reinterpret_cast<std::uint64_t *>(&result);
    peek(p, sizeof(result));
    *p = network2Host(*p);
    return result;
}

flute::ssize_t RingBuffer::peek(void *buffer, flute::ssize_t length) const {
    // assert(length <= readableBytes());
    auto bytesAvaliable = readableBytes();
    length = length > bytesAvaliable ? bytesAvaliable : length;
    if (m_capacity - m_readIndex >= length) {
        std::memcpy(buffer, m_buffer + m_readIndex, length);
    } else {
        std::memcpy(buffer, m_buffer + m_readIndex, m_capacity - m_readIndex);
        std::memcpy(reinterpret_cast<std::uint8_t *>(buffer) + m_capacity - m_readIndex, m_buffer,
                    length + m_readIndex - m_capacity);
    }
    return length;
}

std::int8_t RingBuffer::readInt8() {
    auto result = peekInt8();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, sizeof(result));
    return result;
}

std::int16_t RingBuffer::readInt16() {
    auto result = peekInt16();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, sizeof(result));
    return result;
}

std::int32_t RingBuffer::readInt32() {
    auto result = peekInt32();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, sizeof(result));
    return result;
}

std::int64_t RingBuffer::readInt64() {
    auto result = peekInt64();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, sizeof(result));
    return result;
}

float RingBuffer::readFloat() {
    auto result = peekFloat();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, sizeof(result));
    return result;
}

double RingBuffer::readDouble() {
    auto result = peekDouble();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, sizeof(result));
    return result;
}

flute::ssize_t RingBuffer::read(void *buffer, flute::ssize_t length) {
    auto count = peek(buffer, length);
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, count);
    return count;
}

void RingBuffer::read(ByteBuffer &buffer, flute::ssize_t length) {
    if (buffer.m_capacity < length) {
        buffer.expand(length);
    }
    auto count = read(buffer.m_buffer + buffer.m_writeIndex, length);
    buffer.m_writeIndex += count;
    return;
}

void RingBuffer::append(RingBuffer &buffer) {
    appendInternal(buffer, buffer.readableBytes());
    UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, buffer.readableBytes());
    buffer.m_readIndex = buffer.m_writeIndex = buffer.m_bufferSize = 0;
}

void RingBuffer::append(RingBuffer &buffer, flute::ssize_t length) {
    appendInternal(buffer, length);
    UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, buffer.readableBytes());
    buffer.m_readIndex = buffer.m_writeIndex = buffer.m_bufferSize = 0;
}

void RingBuffer::append(const void *buffer, flute::ssize_t length) {
    if (length > writeableBytes()) {
        // expand buffer
        expand(length);
    }
    if (m_readIndex <= m_writeIndex) {
        if (m_capacity - m_writeIndex >= length) {
            std::memcpy(m_buffer + m_writeIndex, buffer, length);
        } else {
            std::memcpy(m_buffer + m_writeIndex, buffer, m_capacity - m_writeIndex);
            std::memcpy(m_buffer, reinterpret_cast<const std::uint8_t *>(buffer) + m_capacity - m_writeIndex,
                        length + m_writeIndex - m_capacity);
        }
    } else {
        std::memcpy(m_buffer + m_writeIndex, buffer, length);
    }
    UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, length);
}

void RingBuffer::append(ByteBuffer &buffer) {
    auto nbytes = buffer.readableBytes();
    if (nbytes <= 0) {
        return;
    }
    append(buffer.m_buffer + buffer.m_readIndex, nbytes);
    buffer.clear();
}

void RingBuffer::appendInt8(std::int8_t value) { append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value)); }

void RingBuffer::appendInt16(std::int16_t value) {
    value = host2Network(value);
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void RingBuffer::appendInt32(std::int32_t value) {
    value = host2Network(value);
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void RingBuffer::appendInt64(std::int64_t value) {
    value = host2Network(value);
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void RingBuffer::appendFloat(float value) {
    auto p = reinterpret_cast<std::uint32_t *>(&value);
    *p = host2Network(*p);
    append(reinterpret_cast<std::uint8_t *>(p), sizeof(value));
}

void RingBuffer::appendDouble(double value) {
    auto p = reinterpret_cast<std::uint64_t *>(&value);
    *p = host2Network(*p);
    append(reinterpret_cast<std::uint8_t *>(p), sizeof(value));
}

flute::ssize_t RingBuffer::readFromSocket(socket_type descriptor) {
    auto writeableSize = writeableBytes();
    auto bytesAvailable = flute::getByteAvaliableOnSocket(descriptor);
    if (bytesAvailable >= writeableSize) {
        // expand buffer
        expand(bytesAvailable);
    }
    iovec vec[2]{};
    int count = 1;
    if (m_readIndex <= m_writeIndex && m_writeIndex != 0) {
        if (m_capacity - m_writeIndex >= bytesAvailable) {
            vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
            vec[0].iov_len = writeableBytes();
            count = 1;
        } else {
            vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
            vec[0].iov_len = m_capacity - m_writeIndex;
            vec[1].iov_base = reinterpret_cast<char *>(m_buffer);
            vec[1].iov_len = bytesAvailable + m_writeIndex - m_capacity;
            count = 2;
        }
    } else {
        vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
        vec[0].iov_len = bytesAvailable;
        count = 1;
    }
    auto result = flute::readv(descriptor, vec, count);
    if (result > 0) {
        UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, result);
    }
    return result;
}

flute::ssize_t RingBuffer::sendToSocket(socket_type descriptor) {
    auto length = readableBytes();
    iovec vec[2]{};
    int count;
    if (m_capacity - m_readIndex >= length) {
        vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
        vec[0].iov_len = length;
        count = 1;
    } else {
        vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
        vec[0].iov_len = m_capacity - m_readIndex;
        vec[1].iov_base = reinterpret_cast<char *>(m_buffer);
        vec[1].iov_len = length + m_readIndex - m_capacity;
        count = 2;
    }
    auto result = flute::writev(descriptor, vec, count);
    if (result > 0) {
        UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, result);
    }
    return result;
}

flute::ssize_t RingBuffer::readFromSocket(socket_type descriptor, const InetAddress &address) {
    auto writeableSize = writeableBytes();
    auto bytesAvailable = flute::getByteAvaliableOnSocket(descriptor);
    if (bytesAvailable >= writeableSize) {
        // expand buffer
        expand(bytesAvailable);
    }
    iovec vec[2]{};
    int count = 1;
    if (m_readIndex <= m_writeIndex && m_writeIndex != 0) {
        if (m_capacity - m_writeIndex >= bytesAvailable) {
            vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
            vec[0].iov_len = writeableBytes();
            count = 1;
        } else {
            vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
            vec[0].iov_len = m_capacity - m_writeIndex;
            vec[1].iov_base = reinterpret_cast<char *>(m_buffer);
            vec[1].iov_len = bytesAvailable + m_writeIndex - m_capacity;
            count = 2;
        }
    } else {
        vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
        vec[0].iov_len = bytesAvailable;
        count = 1;
    }
    msghdr message{};
    message.msg_name = const_cast<sockaddr*>(address.getSocketAddress());
    message.msg_namelen = static_cast<socklen_t>(address.getSocketLength());
    message.msg_iov = vec;
    message.msg_iovlen = count;
    auto result = flute::recvmsg(descriptor, &message, 0);
    if (result > 0) {
        UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, result);
    }
    return result;
}

flute::ssize_t RingBuffer::sendToSocket(socket_type descriptor, const InetAddress &address) {
    auto length = readableBytes();
    iovec vec[2]{};
    int count;
    if (m_capacity - m_readIndex >= length) {
        vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
        vec[0].iov_len = length;
        count = 1;
    } else {
        vec[0].iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
        vec[0].iov_len = m_capacity - m_readIndex;
        vec[1].iov_base = reinterpret_cast<char *>(m_buffer);
        vec[1].iov_len = length + m_readIndex - m_capacity;
        count = 2;
    }
    msghdr message{};
    message.msg_name = const_cast<sockaddr *>(address.getSocketAddress());
    message.msg_namelen = static_cast<socklen_t>(address.getSocketLength());
    message.msg_iov = vec;
    message.msg_iovlen = count;
    auto result = flute::sendmsg(descriptor, &message, 0);
    if (result > 0) {
        UPDATE_READ_INDEX(m_capacity, m_readIndex, m_writeIndex, m_bufferSize, result);
    }
    return result;
}

void RingBuffer::clear() { m_readIndex = m_writeIndex = m_bufferSize = 0; }

void RingBuffer::getReadableBuffer(std::vector<iovec> &vecs) {
    auto length = readableBytes();
    if (m_capacity - m_readIndex >= length) {
        iovec vec;
        vec.iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
        vec.iov_len = length;
        vecs.push_back(vec);
    } else {
        iovec vec{};
        vec.iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
        vec.iov_len = m_capacity - m_readIndex;
        vecs.push_back(vec);
        vec.iov_base = reinterpret_cast<char *>(m_buffer);
        vec.iov_len = length + m_readIndex - m_capacity;
        vecs.push_back(vec);
    }
}

void RingBuffer::getWriteableBuffer(std::vector<iovec> &vecs) {
    auto writeableSize = writeableBytes();
    if (writeableSize <= 0) {
        // expand buffer
        auto size = m_capacity;
        if (size == 0) {
            size = DETAULT_BUFFER_SIZE;
        }
        expand(size);
    }
    writeableSize = writeableBytes();
    if (m_readIndex <= m_writeIndex && m_writeIndex != 0) {
        iovec vec{};
        vec.iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
        vec.iov_len = m_capacity - m_writeIndex;
        vecs.push_back(vec);
        vec.iov_base = reinterpret_cast<char *>(m_buffer);
        vec.iov_len = writeableSize + m_writeIndex - m_capacity;
        vecs.push_back(vec);
    } else {
        iovec vec{};
        vec.iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
        vec.iov_len = writeableSize;
        vecs.push_back(vec);
    }
}

void RingBuffer::updateWriteIndex(flute::ssize_t length) {
    UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, length);
}

void RingBuffer::expand(flute::ssize_t length) {
    auto capacity = getCapacity(length, m_capacity);
    auto new_buffer = static_cast<std::uint8_t *>(std::realloc(m_buffer, capacity));
    if (!new_buffer) {
        LOG_ERROR << "out of memory";
    } else {
        if (new_buffer != m_buffer) {
            if (readableBytes() > m_capacity - m_readIndex) {
                auto headCnt = readableBytes() + m_readIndex - m_capacity;
                if (headCnt <= capacity - m_capacity) {
                    std::memmove(new_buffer + m_capacity, m_buffer, headCnt);
                } else {
                    std::memmove(new_buffer + m_capacity, m_buffer, capacity - m_capacity);
                    std::memmove(new_buffer, m_buffer + capacity - m_capacity, headCnt + m_capacity - capacity);
                }
            }
        }
        m_buffer = new_buffer;
        m_writeIndex = m_readIndex + m_bufferSize;
        m_capacity = capacity;
    }
}

void RingBuffer::appendInternal(const RingBuffer &buffer, flute::ssize_t length) {
    if (length > writeableBytes()) {
        // expand buffer
        expand(length);
    }
    if (buffer.m_readIndex < buffer.m_writeIndex) {
        append(buffer.m_buffer + buffer.m_readIndex, length);
    } else {
        append(buffer.m_buffer + buffer.m_readIndex, buffer.m_capacity - buffer.m_readIndex);
        append(buffer.m_buffer, length + buffer.m_readIndex - buffer.m_capacity);
    }
}

} // namespace flute
