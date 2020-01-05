//
// Created by why on 2019/12/31.
//

#ifndef FLUTE_DETAIL_POLL_SELECTOR_H
#define FLUTE_DETAIL_POLL_SELECTOR_H

#include <flute/Logger.h>
#include <flute/Selector.h>
#include <flute/flute-config.h>

#include <sys/poll.h>

#include <cassert>
#include <map>

namespace flute {
namespace detail {

class PollSelector : public Selector {
private:
    struct PollSelectorData;

public:
    PollSelector() : m_events(), m_dataMap() {}
    ~PollSelector() = default;

    void addEvent(socket_type descriptor, int old, int events, void* data) override {
        auto temp = old | events;
        auto selectorData = new PollSelectorData();
        selectorData->index = 0;
        selectorData->data = data;
        auto ret = m_dataMap.insert(std::pair<socket_type, PollSelectorData*>(descriptor, selectorData));
        short e = 0;
        if (temp & SelectorEvent::EVENT_READ) e |= POLLIN;
        if (temp & SelectorEvent::EVENT_WRITE) e |= POLLOUT;
        if (!ret.second) {
            // exists
            ret.first->second->data = data;
            auto index = ret.first->second->index;
            auto& pfd = m_events[index];
            pfd.events = e;
            pfd.fd = descriptor;
        } else {
            pollfd pfd = {descriptor, e, 0};
            m_events.push_back(pfd);
            ret.first->second->index = m_events.size() - 1;
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
            auto selectorData = it->second;
            auto iterator = m_events.begin() + selectorData->index;
            std::iter_swap(iterator, m_events.end() - 1);
            auto& tmp = m_events[selectorData->index];
            auto tempIterator = m_dataMap.find(tmp.fd);
            tempIterator->second->index = selectorData->index;
            m_dataMap.erase(it);
            m_events.pop_back();
            delete selectorData;
            return;
        }
        auto& p = it->second;
        p->data = data;
        m_events[p->index].events = e;
    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {
        auto count = ::poll(m_events.data(), m_events.size(), timeout);
        if (count == -1) {
            // auto error = getLastError();
            // LOG_ERROR << "poll error " << error << ":" << formatErrorString(error);
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
            if (it == m_dataMap.end()) {
                LOG_DEBUG << "poll descriptor " << pfd.fd;
            }
            assert(it != m_dataMap.end());
            e.data = it->second->data;
            e.events = 0;
            if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL | POLLIN)) e.events |= SelectorEvent::EVENT_READ;
            if (pfd.revents & POLLOUT) e.events |= SelectorEvent::EVENT_WRITE;
            index += 1;
        }
        return count;
    }

private:
    using size_type = std::vector<pollfd>::size_type;
    struct PollSelectorData {
        size_type index;
        void* data;
    };
    std::vector<pollfd> m_events;
    std::map<socket_type, PollSelectorData*> m_dataMap;
};

} // namespace detail
} // namespace flute

#endif // FLUTE_DETAIL_POLL_SELECTOR_H
