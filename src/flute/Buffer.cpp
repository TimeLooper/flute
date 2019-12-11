/*************************************************************************
 *
 * File Name:  Buffer.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/08 21:31:01
 *
 *************************************************************************/

#include <flute/Buffer.h>

namespace flute {

Buffer::Buffer() : m_chains() {

}

Buffer::~Buffer() {

}

std::int8_t Buffer::peekInt8() const {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    return *(reinterpret_cast<std::int8_t *>(ch->buffer + ch->readIndex));
}

std::int16_t Buffer::peekInt16() const {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    return *(reinterpret_cast<std::int16_t *>(ch->buffer + ch->readIndex));
}

std::int32_t Buffer::peekInt32() const {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    return *(reinterpret_cast<std::int32_t *>(ch->buffer + ch->readIndex));
}

std::int64_t Buffer::peekInt64() const {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    return *(reinterpret_cast<std::int64_t *>(ch->buffer + ch->readIndex));
}

std::int8_t Buffer::readInt8() {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    auto result = *(reinterpret_cast<std::int8_t *>(ch->buffer + ch->readIndex));
    ch->readIndex += sizeof(result);
    return result;
}

std::int16_t Buffer::readInt16() {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    auto result = *(reinterpret_cast<std::int16_t *>(ch->buffer + ch->readIndex));
    ch->readIndex += sizeof(result);
    return result;
}

std::int32_t Buffer::readInt32() {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    auto result = *(reinterpret_cast<std::int32_t *>(ch->buffer + ch->readIndex));
    ch->readIndex += sizeof(result);
    return result;
}

std::int64_t Buffer::readInt64() {
    if (m_chains.empty()) {
        return 0;
    }
    auto ch = m_chains.front();
    auto result = *(reinterpret_cast<std::int64_t *>(ch->buffer + ch->readIndex));
    ch->readIndex += sizeof(result);
    return result;
}

std::string Buffer::readLine() {
    if (m_chains.empty()) {
        return "";
    }
    char temp[65536]{};
    for (const auto ch : m_chains) {

    }
    return "";
}

void Buffer::appendInt8(std::int8_t value) {

}

void Buffer::appendInt16(std::int16_t value) {

}

void Buffer::appendInt32(std::int32_t value) {

}

void Buffer::appendInt64(std::int64_t value) {

}

void Buffer::appendBufferChain(BufferChain* ch) {

}

void Buffer::append(const void* buffer, std::size_t size) {

}

void Buffer::peek(std::uint8_t* buffer, std::size_t length) {
    
}

} // namespace flute
