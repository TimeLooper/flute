//
// Created by why on 2020/01/18.
//

#include <flute/ByteBuffer.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/endian.h>
#include <flute/socket_ops.h>

#include <cstdlib>
#include <cstring>

namespace flute {

static const int DEFAULT_BUFFER_SIZE = 1024;

inline flute::ssize_t getCapacity(flute::ssize_t length, flute::ssize_t capacity) {
    int result = capacity ? capacity : 1;
    while (result < length + capacity) {
        result <<= 1;
    }
    return result;
}

ByteBuffer::ByteBuffer()
    : m_readIndex(0)
    , m_writeIndex(0)
    , m_capacity(DEFAULT_BUFFER_SIZE)
    , m_buffer(static_cast<std::uint8_t *>(std::malloc(sizeof(std::uint8_t) * DEFAULT_BUFFER_SIZE))) {}

ByteBuffer::ByteBuffer(ByteBuffer &&buffer) noexcept { this->swap(buffer); }

ByteBuffer::~ByteBuffer() { std::free(m_buffer); }

ByteBuffer &ByteBuffer::operator=(ByteBuffer &&buffer) noexcept {
    if (this == &buffer) {
        return *this;
    }
    this->swap(buffer);
    return *this;
}

void ByteBuffer::swap(ByteBuffer &buffer) {
    std::swap(m_readIndex, buffer.m_readIndex);
    std::swap(m_writeIndex, buffer.m_writeIndex);
    std::swap(m_capacity, buffer.m_capacity);
    std::swap(m_buffer, buffer.m_buffer);
}

flute::ssize_t ByteBuffer::readableBytes() const { return m_writeIndex - m_readIndex; }

flute::ssize_t ByteBuffer::writeableBytes() const { return m_capacity - m_writeIndex; }

std::int8_t ByteBuffer::peekInt8() const {
    std::int8_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return result;
}

std::int16_t ByteBuffer::peekInt16() const {
    std::int16_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return result;
}

std::int32_t ByteBuffer::peekInt32() const {
    std::int32_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return result;
}

std::int64_t ByteBuffer::peekInt64() const {
    std::int64_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return result;
}

float ByteBuffer::peekFloat() const {
    float result = 0.0f;
    auto p = reinterpret_cast<std::uint32_t *>(&result);
    peek(p, sizeof(result));
    return result;
}

double ByteBuffer::peekDouble() const {
    double result = 0.0f;
    auto p = reinterpret_cast<std::uint64_t *>(&result);
    peek(p, sizeof(result));
    return result;
}

flute::ssize_t ByteBuffer::peek(void *buffer, flute::ssize_t length) const {
    // assert(length <= readableBytes());
    auto bytesAvaliable = readableBytes();
    length = length > bytesAvaliable ? bytesAvaliable : length;
    std::memcpy(buffer, m_buffer + m_readIndex, length);
    return length;
}

std::int8_t ByteBuffer::readInt8() {
    auto result = peekInt8();
    m_readIndex += sizeof(result);
    return result;
}

std::int16_t ByteBuffer::readInt16() {
    auto result = peekInt16();
    m_readIndex += sizeof(result);
    return result;
}

std::int32_t ByteBuffer::readInt32() {
    auto result = peekInt32();
    m_readIndex += sizeof(result);
    return result;
}

std::int64_t ByteBuffer::readInt64() {
    auto result = peekInt64();
    m_readIndex += sizeof(result);
    return result;
}

float ByteBuffer::readFloat() {
    auto result = peekFloat();
    m_readIndex += sizeof(result);
    return result;
}

double ByteBuffer::readDouble() {
    auto result = peekDouble();
    m_readIndex += sizeof(result);
    return result;
}

flute::ssize_t ByteBuffer::read(void *buffer, flute::ssize_t length) {
    auto count = peek(buffer, length);
    m_readIndex += count;
    return count;
}

void ByteBuffer::append(ByteBuffer &buffer) {
    appendInternal(buffer, buffer.readableBytes());
    m_writeIndex += buffer.readableBytes();
    buffer.m_readIndex = buffer.m_writeIndex = 0;
}

void ByteBuffer::append(ByteBuffer &buffer, flute::ssize_t length) {
    appendInternal(buffer, length);
    m_writeIndex += buffer.readableBytes();
    buffer.m_readIndex = buffer.m_writeIndex = 0;
}

void ByteBuffer::append(const void *buffer, flute::ssize_t length) {
    if (length > writeableBytes()) {
        // expand buffer
        expand(length);
    }
    std::memcpy(m_buffer + m_writeIndex, buffer, length);
    m_writeIndex += length;
}

void ByteBuffer::appendInt8(std::int8_t value) { append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value)); }

void ByteBuffer::appendInt16(std::int16_t value) {    
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void ByteBuffer::appendInt32(std::int32_t value) {
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void ByteBuffer::appendInt64(std::int64_t value) {
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void ByteBuffer::appendFloat(float value) {
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void ByteBuffer::appendDouble(double value) {
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

flute::ssize_t ByteBuffer::readFromSocket(socket_type descriptor) {
    auto writeableSize = writeableBytes();
    auto bytesAvailable = flute::getByteAvaliableOnSocket(descriptor);
    if (bytesAvailable >= writeableSize) {
        // expand buffer
        expand(bytesAvailable);
    }
    iovec vec{};
    int count = 1;
    vec.iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
    vec.iov_len = bytesAvailable;
    count = 1;
    auto result = flute::readv(descriptor, &vec, count);
    if (result > 0) {
        m_writeIndex += result;
    }
    return result;
}

flute::ssize_t ByteBuffer::sendToSocket(socket_type descriptor) {
    auto length = readableBytes();
    iovec vec{};
    int count;
    vec.iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
    vec.iov_len = length;
    count = 1;
    auto result = flute::writev(descriptor, &vec, count);
    if (result > 0) {
        m_readIndex += result;
    }
    return result;
}

flute::ssize_t ByteBuffer::readFromSocket(socket_type descriptor, InetAddress &address) {
    auto writeableSize = writeableBytes();
    auto bytesAvailable = flute::getByteAvaliableOnSocket(descriptor);
    if (bytesAvailable >= writeableSize) {
        // expand buffer
        expand(bytesAvailable);
    }
    iovec vec{};
    int count = 1;
    vec.iov_base = reinterpret_cast<char *>(m_buffer + m_writeIndex);
    vec.iov_len = bytesAvailable;
    msghdr message{};
    message.msg_name = address.getSocketAddress();
    message.msg_namelen = static_cast<socklen_t>(address.getSocketLength());
    message.msg_iov = &vec;
    message.msg_iovlen = count;
    auto result = flute::recvmsg(descriptor, &message, 0);
    if (result > 0) {
        m_writeIndex += result;
    }
    return result;
}

flute::ssize_t ByteBuffer::sendToSocket(socket_type descriptor, const InetAddress &address) {
    auto length = readableBytes();
    iovec vec{};
    int count;
    vec.iov_base = reinterpret_cast<char *>(m_buffer + m_readIndex);
    vec.iov_len = length;
    count = 1;
    msghdr message{};
    message.msg_name = const_cast<sockaddr *>(address.getSocketAddress());
    message.msg_namelen = static_cast<socklen_t>(address.getSocketLength());
    message.msg_iov = &vec;
    message.msg_iovlen = count;
    auto result = flute::sendmsg(descriptor, &message, 0);
    if (result > 0) {
        m_readIndex += result;
    }
    return result;
}

void ByteBuffer::clear() { m_readIndex = m_writeIndex = 0; }

void ByteBuffer::expand(flute::ssize_t length) {
    auto capacity = getCapacity(length, m_capacity);
    auto new_buffer = static_cast<std::uint8_t *>(std::realloc(m_buffer, capacity));
    if (!new_buffer) {
        LOG_ERROR << "out of memory";
    } else {
        auto nbytes = readableBytes();
        if (length > writeableBytes()) {
            std::memmove(new_buffer, new_buffer + m_readIndex, nbytes);
        }
        m_buffer = new_buffer;
        m_readIndex = 0;
        m_writeIndex = nbytes;
        m_capacity = capacity;
    }
}

void ByteBuffer::appendInternal(const ByteBuffer &buffer, flute::ssize_t length) {
    if (length > writeableBytes()) {
        // expand buffer
        expand(length);
    }
    append(buffer.m_buffer + buffer.m_readIndex, length);
}

} // namespace flute
