/*************************************************************************
 *
 * File Name:  Timer.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/03 23:51:38
 *
 *************************************************************************/

#pragma once

#include <flute/internal.h>

#include <cstdint>
#include <functional>

namespace flute {

class Timer {
public:
    Timer(std::function<void()>&& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount)
        , index(-1)
        , startTime(currentMilliseconds())
        , delay(delay)
        , callback(std::move(callback)) {
    }
    Timer(const std::function<void()>& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount), index(-1), startTime(currentMilliseconds()), delay(delay), callback(callback) {
    }

    int loopCount;
    int index;
    std::int64_t startTime;
    std::int64_t delay;
    std::function<void()> callback;
};

} // namespace flute