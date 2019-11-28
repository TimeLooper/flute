/*************************************************************************
 *
 * File Name:  noncopyable.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26
 *
 *************************************************************************/

#pragma once

namespace flute {

class noncopyable {
public:
    noncopyable() = default;
    ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;
    const noncopyable &operator=(const noncopyable &) = delete;
};

} // namespace flute