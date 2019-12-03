/*************************************************************************
 *
 * File Name:  internal.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/03 23:52:31
 *
 *************************************************************************/

#pragma once

#include <cstdint>
#include <chrono>

namespace flute {

inline std::int64_t currentMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

}