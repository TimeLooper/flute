//
// Created by why on 2019/12/31.
//

#ifndef FLUTE_DETAIL_POLL_SELECTOR_H
#define FLUTE_DETAIL_POLL_SELECTOR_H

#include <flute/flute-config.h>
#include <flute/Logger.h>
#include <flute/Selector.h>

#include <sys/poll.h>

#include <cassert>
#include <map>

namespace flute {
namespace detail {

class PollSelector : public Selector {
public:
    PollSelector() : m_events(), m_dataMap() {}
    ~PollSelector() = default;

    void addEvent(socket_type descriptor, int old, int events, void* data) override {
        auto temp = old | events;
        auto ret = m_dataMap.insert(std::pair<socket_type, std::pair<size_type, void*>>(descriptor, std::pair<size_type, void*>(0, data)));
        short e = 0;
        if (temp & SelectorEvent::EVENT_READ) e |= POLLIN;
        if (temp & SelectorEvent::EVENT_WRITE) e |= POLLOUT;
        if (!ret.second) {
            // exists
            ret.first->second.second = data;
            auto index = ret.first->second.first;
            auto& temp = m_events[index];
            temp.events = e;
            temp.fd = descriptor;
        } else {
            pollfd pfd = { descriptor, e, 0 };
            m_events.push_back(pfd);
            ret.first->second.first = m_events.size() - 1;
        }
    }

    void removeEvent(socket_type descriptor, int old, int events, void* data) override {
        auto temp = old & (~events);
        short e = 0;
        if (temp & SelectorEvent::EVENT_READ) e |= POLLIN;
        if (temp & SelectorEvent::EVENT_WRITE) e |= POLLOUT;
        auto it = m_dataMap.find(descriptor);
        if (it == m_dataMap.end()) {
            return;
        }
        if (temp == SelectorEvent::EVENT_NONE) {
            m_events.erase(it->second.first + m_events.begin());
            m_dataMap.erase(it);
            return;
        }
        auto& p = it->second;
        p.second = data;
        m_events[p.first].events = e;
    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {
        auto count = ::poll(m_events.data(), m_events.size(), timeout);
        if (count == -1) {
            LOG_ERROR << "poll error " << errno << ":" << std::strerror(errno);
            return -1;
        }
        if (count > 0 && static_cast<std::size_t>(count) > events.size()) {
            events.resize(count);
        }
        auto index = 0;
        for (const auto& pfd : m_events) {
            if (pfd.revents == 0) {
                continue;
            }
            auto& e = events[index];
            auto it = m_dataMap.find(pfd.fd);
            assert(it != m_dataMap.end());
            e.data = it->second.second;
            e.events = 0;
            if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) e.events |= SelectorEvent::EVENT_READ;
            if (pfd.revents & POLLIN) e.events |= SelectorEvent::EVENT_READ;
            if (pfd.revents & POLLOUT) e.events |= SelectorEvent::EVENT_WRITE;
            index += 1;
        }
        return count;
    }

private:
    using size_type = std::vector<pollfd>::size_type;
    std::vector<pollfd> m_events;
    std::map<socket_type, std::pair<size_type, void*>> m_dataMap;
};

} // namespace detail
} // namespace flute

#endif // FLUTE_DETAIL_POLL_SELECTOR_H
