/*************************************************************************
 *
 * File Name:  noncopyable.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26 01:16:53
 *
 *************************************************************************/

#pragma once

namespace flute {

class noncopyable {
public:
    noncopyable() = default;
    ~noncopyable() = default;

    noncopyable(noncopyable&&) = delete;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
    noncopyable& operator=(noncopyable&&) = delete;
};

} // namespace flute