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

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace flute {

static const int DEFAULT_BUFFER_SIZE = 4096;

#define UPDATE_READ_INDEX(capacity, readIndex, bufferSize, size) \
    do {                                                         \
        readIndex += size;                                       \
        readIndex &= capacity - 1;                               \
        bufferSize -= size;                                      \
    } while (0)

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

std::size_t Buffer::readableBytes() const {
    return m_bufferSize;
}

std::size_t Buffer::writeableBytes() const {
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
    auto length = m_lineSeparator.length();
    std::size_t index = 0;
    char buffer[65536];
    while (true) {
        bool match = true;
        for (auto i = 0; i < length; ++i) {
            if (*(m_buffer + ((m_readIndex + index + i) & (m_capacity - 1))) != temp[i]) {
                match = false;
            }
        }
        if (match) {
            break;
        }
        *(buffer + index) = *(m_buffer + ((m_readIndex + index) & (m_capacity - 1)));
        index += 1;
    }
    *(buffer + index) = '\0';
    return buffer;
}

void Buffer::peek(std::uint8_t *buffer, std::size_t length) const {
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

void Buffer::read(std::uint8_t *buffer, std::size_t length) {
    peek(buffer, length);
    UPDATE_READ_INDEX(m_capacity, m_readIndex, m_bufferSize, length);
}

void Buffer::append(const std::uint8_t *buffer, std::size_t length) {
    if (length > m_bufferSize) {
        // expand buffer
        auto capacity = getCapacity();
        auto new_buffer = static_cast<std::uint8_t *>(std::realloc(m_buffer, capacity));
        if (!new_buffer) {
            LOG_ERROR << "out of memory";
        } else {
            auto headCnt = readableBytes() + m_readIndex - m_capacity;
            if (headCnt <= capacity - m_capacity) {
                std::memmove(new_buffer + m_capacity, m_buffer, headCnt);
            } else {
                std::memmove(new_buffer + m_capacity, m_buffer, capacity - m_capacity);
                std::memmove(new_buffer, m_buffer + capacity - m_capacity, headCnt + m_capacity - capacity);
            }
            std::free(m_buffer);
            m_buffer = new_buffer;
            m_writeIndex += headCnt;
            m_capacity = capacity;
            m_writeIndex &= capacity - 1;
        }
    }
    if (m_readIndex < m_writeIndex) {
        if (m_capacity - m_writeIndex < length) {
            std::memcpy(m_buffer + m_writeIndex, buffer, length);
        } else {
            std::memcpy(m_buffer + m_writeIndex, buffer, m_capacity - m_writeIndex);
            std::memcpy(m_buffer + m_capacity, buffer + m_capacity - m_writeIndex, length + m_writeIndex - m_capacity);
        }
    } else {
        std::memcpy(m_buffer + m_writeIndex, buffer, length);
    }
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

std::size_t Buffer::getCapacity() {
    return 0;
}

} // namespace flute
