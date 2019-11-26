/*************************************************************************
 *
 * File Name:  epoll_event_loop.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26 02:25:50
 *
 *************************************************************************/

#pragma once

#include <flute/noncopyable.hpp>

namespace flute {
namespace impl {

class epoll_event_loop : private noncopyable {
public:
    epoll_event_loop();
    ~epoll_event_loop();

};

} // namespace detail
} // namespace flute