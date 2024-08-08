//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_SELECTOR_H
#define FLUTE_SELECTOR_H

#include <flute/EventLoop.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <vector>

namespace flute {

struct SelectorEvent {
    static const int EVENT_NONE = 0x0;
    static const int EVENT_READ = 0x1;
    static const int EVENT_WRITE = 0x2;
    static const int EVENT_ET = 0x4;

    int events;
    void* data;
    SelectorEvent();
};

class Selector : private noncopyable {
public:
    FLUTE_API_DECL Selector() = default;
    FLUTE_API_DECL virtual ~Selector() = default;

    FLUTE_API_DECL virtual void addEvent(socket_type descriptor, int old, int events, void* data) = 0;
    FLUTE_API_DECL virtual void removeEvent(socket_type descriptor, int old, int events, void* data) = 0;
    FLUTE_API_DECL virtual int select(std::vector<SelectorEvent>& events, int timeout) = 0;

    FLUTE_API_DECL static Selector* createSelector();
};

} // namespace flute

#endif // FLUTE_SELECTOR_H
