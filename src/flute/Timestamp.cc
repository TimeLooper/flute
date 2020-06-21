//
// Created by why on 2020/01/06.
//

#include <flute/Timestamp.h>

#include <flute/internal.h>

namespace flute {

Timestamp Timestamp::now() {
    return Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count());
}

} // namespace flute