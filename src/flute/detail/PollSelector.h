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
        auto ret = m_dataMap.insert(std::pair<socket_type, std::pair<size_type, void*>>(descriptor, std::pair<size_type, void*>(0, data)));
        pollfd pfd = {descriptor, 0, 0};
        if (temp & SelectorEvent::EVENT_READ) pfd.events |= POLLIN;
        if (temp & SelectorEvent::EVENT_WRITE) pfd.events |= POLLOUT;
        if (!ret.second) {
            // exists
            ret.first->second.second = data;
            auto index = ret.first->second.first;
            auto& temp = m_events[index];
            temp.events = pfd.events;
            temp.fd = descriptor;
        } else {
            m_events.push_back(pfd);
            ret.first->second.first = m_events.size() - 1;
        }
    }

    void removeEvent(socket_type descriptor, int old, int events, void* data) override {
        auto temp = old & (~events);
        auto ret = m_dataMap.insert(std::pair<socket_type, std::pair<size_type, void*>>(descriptor, std::pair<size_type, void*>(0, data)));
        pollfd pfd = {descriptor, 0, 0};
        if (temp & SelectorEvent::EVENT_READ) pfd.events |= POLLIN;
        if (temp & SelectorEvent::EVENT_WRITE) pfd.events |= POLLOUT;
        if (!ret.second) {
            // exists
            ret.first->second.second = data;
            auto index = ret.first->second.first;
            auto& temp = m_events[index];
            temp.events = pfd.events;
            temp.fd = descriptor;
        } else {
            m_events.push_back(pfd);
            ret.first->second.first = m_events.size() - 1;
        }
    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {

    }

private:
    using size_type = std::vector<pollfd>::size_type;
    std::vector<pollfd> m_events;
    std::map<socket_type, std::pair<size_type, void*>> m_dataMap;
};

} // namespace detail
} // namespace flute

#endif