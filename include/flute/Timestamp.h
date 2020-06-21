//
// Created by why on 2020/06/17.
//

#ifndef FLUTE_TIMESTAMP_H
#define FLUTE_TIMESTAMP_H

#include <flute/config.h>
#include <flute/copyable.h>

#include <cstdint>
#include <algorithm>

namespace flute {

class Timestamp : private copyable {
public:
    FLUTE_API_DECL Timestamp() : m_microSeconds(0) {}

    FLUTE_API_DECL explicit Timestamp(std::int64_t microSeconds) : m_microSeconds(microSeconds) {}

    FLUTE_API_DECL ~Timestamp();

    FLUTE_API_DECL bool operator==(const Timestamp& rhs) { return this->m_microSeconds == rhs.m_microSeconds; }

    FLUTE_API_DECL bool operator!=(const Timestamp& rhs) { return !(*this == rhs); }

    FLUTE_API_DECL bool operator<(const Timestamp& rhs) { return this->m_microSeconds < rhs.m_microSeconds; }

    FLUTE_API_DECL bool operator>(const Timestamp& rhs) { return this->m_microSeconds > rhs.m_microSeconds; }

    FLUTE_API_DECL Timestamp& operator=(const Timestamp& rhs) {
        this->m_microSeconds = rhs.m_microSeconds;
        return *this;
    }

    FLUTE_API_DECL Timestamp& operator=(Timestamp&& rhs) {
        std::swap(this->m_microSeconds, rhs.m_microSeconds);
        return *this;
    }

    FLUTE_API_DECL std::int64_t getTime() const { return m_microSeconds; }

    FLUTE_API_DECL void setTime(std::int64_t time) { m_microSeconds = time; }

    static Timestamp now();

private:
    std::int64_t m_microSeconds;
};

} // namespace flute

#endif // FLUTE_TIMESTAMP_H