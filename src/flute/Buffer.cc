//
// Created by why on 2019/12/30.
//

#include <flute/Buffer.h>
#include <flute/Logger.h>
#include <flute/endian.h>
#include <flute/socket_ops.h>

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace flute {

static const int DEFAULT_BUFFER_SIZE = 1024;

#define UPDATE_READ_INDEX(capacity, readIndex, bufferSize, size) \
    do {                                                         \
        readIndex += size;                                       \
        readIndex &= capacity - 1;                               \
        bufferSize -= size;                                      \
    } while (0)

#define UPDATE_WRITE_INDEX(capacity, writeIndex, bufferSize, size) \
    do {                                                           \
        writeIndex += size;                                        \
        writeIndex &= capacity - 1;                                \
        bufferSize += size;                                        \
    } while (0)

inline std::int32_t getCapacity(std::int32_t length, std::int32_t capacity) {
    int result = capacity ? capacity : 1;
    while (result < length + capacity) {
        result <<= 1;
    }
    return result;
}

Buffer::Buffer()
    : m_readIndex(0)
    , m_writeIndex(0)
    , m_bufferSize(0)
    , m_capacity(DEFAULT_BUFFER_SIZE)
    , m_buffer(static_cast<std::uint8_t *>(std::malloc(sizeof(std::uint8_t) * DEFAULT_BUFFER_SIZE)))
    , m_lineSeparator("\r\n") {}

Buffer::Buffer(const Buffer &buffer) : Buffer() { appendInternal(buffer); }

Buffer::Buffer(Buffer &&buffer) : Buffer() { this->swap(buffer); }

Buffer::~Buffer() { std::free(m_buffer); }

Buffer &Buffer::operator=(const Buffer &buffer) {
    this->m_readIndex = this->m_writeIndex = this->m_bufferSize = 0;
    appendInternal(buffer);
    return *this;
}

Buffer &Buffer::operator=(Buffer &&buffer) {
    this->swap(buffer);
    return *this;
}

void Buffer::swap(Buffer &buf) {
    std::swap(m_readIndex, buf.m_readIndex);
    std::swap(m_writeIndex, buf.m_writeIndex);
    std::swap(m_bufferSize, buf.m_bufferSize);
    std::swap(m_capacity, buf.m_capacity);
    std::swap(m_buffer, buf.m_buffer);
    m_lineSeparator.swap(buf.m_lineSeparator);
}

flute::ssize_t Buffer::readableBytes() const { return m_bufferSize; }

flute::ssize_t Buffer::writeableBytes() const { return m_capacity - m_bufferSize; }

std::int8_t Buffer::peekInt8() const {
    std::int8_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return result;
}

std::int16_t Buffer::peekInt16() const {
    std::int16_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return flute::network2Host(result);
}

std::int32_t Buffer::peekInt32() const {
    std::int32_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return flute::network2Host(result);
}

std::int64_t Buffer::peekInt64() const {
    std::int64_t result = 0;
    peek(reinterpret_cast<std::uint8_t *>(&result), sizeof(result));
    return flute::network2Host(result);
}

std::string Buffer::peekLine() const {
    auto temp = m_lineSeparator.c_str();
    flute::ssize_t length = static_cast<flute::ssize_t>(m_lineSeparator.length());
    auto bytesAvailable = readableBytes();
    flute::ssize_t index = 0;
    flute::ssize_t idx = 0;
    std::stringstream ss;
    while (true) {
        ss << *(m_buffer + ((m_readIndex + index) & (m_capacity - 1)));
        if (*(m_buffer + ((m_readIndex + index) & (m_capacity - 1))) == temp[idx]) {
            idx += 1;
        } else {
            idx = 0;
        }
        if (idx >= length) {
            break;
        }
        if (index >= bytesAvailable) {
            return "";
        }
        index += 1;
    }
    return ss.str();
}

void Buffer::peek(void *buffer, flute::ssize_t length) const {
    assert(length <= readableBytes());
    if (m_capacity - m_readIndex >= length) {
        std::memcpy(buffer, m_buffer + m_readIndex, length);
    } else {
        std::memcpy(buffer, m_buffer + m_readIndex, m_capacity - m_readIndex);
        std::memcpy(reinterpret_cast<std::uint8_t *>(buffer) + m_capacity - m_readIndex, m_buffer,
                    length + m_readIndex - m_capacity);
    }
}

std::int8_t Buffer::readInt8() {
    auto result = peekInt8();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, sizeof(result));
    return result;
}

std::int16_t Buffer::readInt16() {
    auto result = peekInt16();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, sizeof(result));
    return result;
}

std::int32_t Buffer::readInt32() {
    auto result = peekInt32();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, sizeof(result));
    return result;
}

std::int64_t Buffer::readInt64() {
    auto result = peekInt64();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, sizeof(result));
    return result;
}

std::string Buffer::readLine() {
    auto result = peekLine();
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, static_cast<flute::ssize_t>(result.length()));
    return result;
}

void Buffer::read(void *buffer, flute::ssize_t length) {
    peek(buffer, length);
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, length);
}

void Buffer::append(Buffer &buffer) {
    appendInternal(buffer);
    UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, buffer.readableBytes());
    buffer.m_readIndex = buffer.m_writeIndex = buffer.m_bufferSize = 0;
}

void Buffer::append(const void *buffer, flute::ssize_t length) {
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

void Buffer::appendInt8(std::int8_t value) { append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value)); }

void Buffer::appendInt16(std::int16_t value) {
    value = host2Network(value);
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void Buffer::appendInt32(std::int32_t value) {
    value = host2Network(value);
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void Buffer::appendInt64(std::int64_t value) {
    value = host2Network(value);
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

void Buffer::setLineSeparator(std::string &&separator) { m_lineSeparator = std::move(separator); }

void Buffer::setLineSeparator(const std::string &separator) { m_lineSeparator = separator; }

const std::string &Buffer::getLineSeparator() const { return m_lineSeparator; }

flute::ssize_t Buffer::readFromSocket(socket_type descriptor) {
    auto writeableSize = writeableBytes();
    auto bytesAvailable = flute::getByteAvaliableOnSocket(descriptor);
    if (bytesAvailable >= writeableSize) {
        // expand buffer
        expand(bytesAvailable);
    }
    iovec vec[2]{};
    int count = 1;
    if (m_readIndex <= m_writeIndex) {
        if (m_capacity - m_writeIndex >= bytesAvailable) {
            vec[0].iov_base = m_buffer + m_writeIndex;
            vec[0].iov_len = writeableBytes();
            count = 1;
        } else {
            vec[0].iov_base = m_buffer + m_writeIndex;
            vec[0].iov_len = m_capacity - m_writeIndex;
            vec[1].iov_base = m_buffer;
            vec[1].iov_len = bytesAvailable + m_writeIndex - m_capacity;
            count = 2;
        }
    } else {
        vec[0].iov_base = m_buffer + m_writeIndex;
        vec[0].iov_len = bytesAvailable;
        count = 1;
    }
    auto result = flute::readv(descriptor, vec, count);
    if (result > 0) {
        UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, result);
    }
    return result;
}

flute::ssize_t Buffer::sendToSocket(socket_type descriptor) {
    auto length = readableBytes();
    iovec vec[2]{};
    int count;
    if (m_capacity - m_readIndex >= length) {
        vec[0].iov_base = m_buffer + m_readIndex;
        vec[0].iov_len = length;
        count = 1;
    } else {
        vec[0].iov_base = m_buffer + m_readIndex;
        vec[0].iov_len = m_capacity - m_readIndex;
        vec[1].iov_base = m_buffer;
        vec[1].iov_len = length + m_readIndex - m_capacity;
        count = 2;
    }
    auto result = flute::writev(descriptor, vec, count);
    if (result > 0) {
        UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, result);
    }
    return result;
}

void Buffer::clear() { m_readIndex = m_writeIndex = m_bufferSize = 0; }

void Buffer::expand(flute::ssize_t length) {
    auto capacity = getCapacity(length, m_capacity);
    auto new_buffer = static_cast<std::uint8_t *>(std::realloc(m_buffer, capacity));
    if (!new_buffer) {
        LOG_ERROR << "out of memory";
    } else {
        if (readableBytes() > m_capacity - m_readIndex) {
            auto headCnt = readableBytes() + m_readIndex - m_capacity;
            if (headCnt <= capacity - m_capacity) {
                std::memmove(new_buffer + m_capacity, m_buffer, headCnt);
            } else {
                std::memmove(new_buffer + m_capacity, m_buffer, capacity - m_capacity);
                std::memmove(new_buffer, m_buffer + capacity - m_capacity, headCnt + m_capacity - capacity);
            }
        }
        m_buffer = new_buffer;
        m_writeIndex = m_readIndex + m_bufferSize;
        m_capacity = capacity;
    }
}

void Buffer::appendInternal(const Buffer &buffer) {
    auto length = buffer.readableBytes();
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
