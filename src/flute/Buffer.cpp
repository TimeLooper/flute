/*************************************************************************
 *
 * File Name:  Buffer.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/08 21:31:01
 *
 *************************************************************************/

#include <flute/Buffer.h>

#include <cassert>
#include <cstdlib>

namespace flute {

static const int DEFAULT_BUFFER_SIZE = 4096;

Buffer::Buffer() : m_readIndex(0), m_writeIndex(0), m_capacity(DEFAULT_BUFFER_SIZE), m_buffer(static_cast<std::uint8_t *>(std::malloc(sizeof(std::uint8_t) * DEFAULT_BUFFER_SIZE))) {

}

Buffer::~Buffer() {
    std::free(m_buffer);
}

std::size_t Buffer::readableBytes() const {
    return m_writeIndex - m_readIndex;
}

std::int8_t Buffer::peekInt8() const {
    assert(readableBytes() >= sizeof(std::int8_t));
    return *reinterpret_cast<std::int8_t *>(m_buffer + m_readIndex);
}

std::int16_t Buffer::peekInt16() const {
    assert(readableBytes() >= sizeof(std::int16_t));
    auto result = *reinterpret_cast<std::int16_t *>(m_buffer + m_readIndex);
    return result;
}

std::int32_t Buffer::peekInt32() const {
    assert(readableBytes() >= sizeof(std::int32_t));
    auto result = *reinterpret_cast<std::int32_t *>(m_buffer + m_readIndex);
    return result;
}

std::int64_t Buffer::peekInt64() const {
    assert(readableBytes() >= sizeof(std::int64_t));
    auto result = *reinterpret_cast<std::int64_t *>(m_buffer + m_readIndex);
    return result;
}

std::int8_t Buffer::readInt8() {
    auto result = peekInt8();
    m_readIndex += sizeof(result);
    return result;
}

std::int16_t Buffer::readInt16() {
    auto result = peekInt16();
    m_readIndex += sizeof(result);
    return result;
}

std::int32_t Buffer::readInt32() {
    auto result = peekInt32();
    m_readIndex += sizeof(result);
    return result;
}

std::int64_t Buffer::readInt64() {
    auto result = peekInt64();
    m_readIndex += sizeof(result);
    return result;
}

std::string Buffer::readLine() {
    
}

void Buffer::append(const std::uint8_t* buffer, std::size_t length) {

}

void Buffer::appendInt8(std::int8_t value) {

}

void Buffer::appendInt16(std::int16_t value) {

}

void Buffer::appendInt32(std::int32_t value) {

}

void Buffer::appendInt64(std::int64_t value) {

}

} // namespace flute
