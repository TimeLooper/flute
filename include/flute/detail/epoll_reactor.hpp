/*************************************************************************
 *
 * File Name:  epoll_reactor.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26 02:25:50
 *
 *************************************************************************/

#pragma once

#include <flute/noncopyable.hpp>

namespace flute {
namespace detail {

class epoll_reactor : private noncopyable {
public:
    epoll_reactor();
    ~epoll_reactor();
};

} // namespace detail
} // namespace flute