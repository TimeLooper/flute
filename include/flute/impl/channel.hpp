/*************************************************************************
 *
 * File Name:  channel.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/27 00:50:01
 *
 *************************************************************************/

#pragma once

#include <flute/config.hpp>
#include <flute/socket_types.hpp>

namespace flute {
namespace impl {

class channel {
public:
    channel(socket_type descriptor) : m_descriptor(descriptor) {}
    ~channel();

private:
    const socket_type m_descriptor;
    short m_events;
};

} // namespace impl
} // namespace flute