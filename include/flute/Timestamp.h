//
// Created by why on 2020/06/17.
//

#ifndef FLUTE_TIMESTAMP_H
#define FLUTE_TIMESTAMP_H

#include <flute/config.h>
#include <flute/copyable.h>

namespace flute {

class Timestamp : private copyable {
public:
    FLUTE_API_DECL Timestamp();

    FLUTE_API_DECL ~Timestamp();

private:
    std::int64_t m_timestamp;
};

} // namespace flute

#endif // FLUTE_TIMESTAMP_H