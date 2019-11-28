/*************************************************************************
 *
 * File Name:  Reactor.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#pragma once

#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/socket_types.h>

#include <cstdint>
#include <vector>

namespace flute {
struct FileEvent;

class Reactor : private noncopyable {
public:
    Reactor() = default;
    virtual ~Reactor() = default;

    virtual void add(socket_type fd, int old, int event, void* data) = 0;
    virtual void remove(socket_type fd, int old, int event, void* data) = 0;
    virtual int wait(std::vector<FileEvent>& events, int timeout) = 0;
};

Reactor* createReactor();

} // namespace flute