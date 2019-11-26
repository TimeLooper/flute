/*************************************************************************
 *
 * File Name:  epoll_event_loop.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26 02:25:50
 *
 *************************************************************************/

#pragma once

#include <flute/config.hpp>
#include <flute/noncopyable.hpp>
#include <flute/impl/channel.hpp>

namespace flute {
namespace impl {

class epoll_event_loop : private noncopyable {
public:
    epoll_event_loop();
    ~epoll_event_loop();

    void registerChannel(channel* channel);
};

#ifdef FLUTE_HEADER_ONLY
#include <flute/impl/epoll_event_loop.inl>
#endif
} // namespace detail
} // namespace flute