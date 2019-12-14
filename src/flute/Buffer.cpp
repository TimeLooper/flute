/*************************************************************************
 *
 * File Name:  Buffer.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/08 21:31:01
 *
 *************************************************************************/

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

static const int DEFAULT_BUFFER_SIZE = 4096;

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
    , m_lineSeparator("\r\n") {
}

Buffer::~Buffer() {
    std::free(m_buffer);
}

std::int32_t Buffer::readableBytes() const {
    return m_bufferSize;
}

std::int32_t Buffer::writeableBytes() const {
    return m_capacity - m_bufferSize;
}

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
	std::int32_t length = m_lineSeparator.length();
    auto bytesAvailable = readableBytes();
	std::int32_t index = 0;
	std::int32_t idx = 0;
    std::stringstream ss;
    while (true) {
        ss << *(m_buffer + ((m_readIndex + index) & (m_capacity - 1)));
        if (*(m_buffer + ((m_readIndex + index) & (m_capacity - 1))) == temp[idx]) {
            idx += 1;
        } else {
            idx = 0;
        }
        if (idx >= length || index >= bytesAvailable) {
            break;
        }
        index += 1;
    }
    return ss.str();
}

void Buffer::peek(std::uint8_t *buffer, std::int32_t length) const {
    assert(length <= readableBytes());
    if (m_capacity - m_readIndex >= length) {
        std::memcpy(buffer, m_buffer + m_readIndex, length);
    } else {
        std::memcpy(buffer, m_buffer + m_readIndex, m_capacity - m_readIndex);
        std::memcpy(buffer + m_capacity - m_readIndex, m_buffer, length + m_readIndex - m_capacity);
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
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, result.length());
    return result;
}

void Buffer::read(std::uint8_t *buffer, std::int32_t length) {
    peek(buffer, length);
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, length);
}

void Buffer::append(const std::uint8_t *buffer, std::int32_t length) {
    if (length > writeableBytes()) {
        // expand buffer
        expand(length);
    }
    if (m_readIndex <= m_writeIndex) {
        if (m_capacity - m_writeIndex >= length) {
            std::memcpy(m_buffer + m_writeIndex, buffer, length);
        } else {
            std::memcpy(m_buffer + m_writeIndex, buffer, m_capacity - m_writeIndex);
            std::memcpy(m_buffer, buffer + m_capacity - m_writeIndex, length + m_writeIndex - m_capacity);
        }
    } else {
        std::memcpy(m_buffer + m_writeIndex, buffer, length);
    }
    UPDATE_WRITE_INDEX(m_capacity, m_writeIndex, m_bufferSize, length);
}

void Buffer::appendInt8(std::int8_t value) {
    append(reinterpret_cast<std::uint8_t *>(&value), sizeof(value));
}

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

void Buffer::setLineSeparator(std::string &&separator) {
    m_lineSeparator = std::move(separator);
}

void Buffer::setLineSeparator(const std::string &separator) {
    m_lineSeparator = separator;
}

const std::string &Buffer::getLineSeparator() const {
    return m_lineSeparator;
}

std::int32_t Buffer::readFromSocket(socket_type descriptor) {
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
            vec[0].iov_len = writeableSize;
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

std::int32_t Buffer::sendToSocket(socket_type descriptor) {
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

void Buffer::expand(std::int32_t length) {
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
        if (new_buffer != m_buffer) {
            std::free(m_buffer);
        }
        m_buffer = new_buffer;
        m_writeIndex = m_readIndex + m_bufferSize;
        m_capacity = capacity;
    }
}

} // namespace flute
