//
// Created by why on 2019/12/31.
//

#ifndef FLUTE_POLL_SELECTOR_H
#define FLUTE_POLL_SELECTOR_H

#include <flute/flute-config.h>
#include <flute/Selector.h>

#include <sys/poll.h>
#include <map>
#include <set>

namespace flute {
namespace detail {

class PollSelector : public Selector {
public:
    PollSelector() : m_events(), m_dataMap() {}
    ~PollSelector() = default;

    void addEvent(socket_type descriptor, int old, int events, void* data) override {
        auto temp = old | events;
        auto it = m_dataMap.find(descriptor);
    }

    void removeEvent(socket_type descriptor, int old, int events, void* data) override {

    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {

    }

private:
    // std::vector<pollfd> m_events;
    std::set<std::pair<socket_type, void*>> m_events;
};

} // namespace detail
} // namespace flute

#endif