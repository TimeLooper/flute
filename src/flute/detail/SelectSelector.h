//
// Created by why on 2019/12/31.
//

#ifndef FLUTE_DETAIL_SELECT_SELECTOR_H
#define FLUTE_DETAIL_SELECT_SELECTOR_H

#include <flute/Selector.h>
#include <flute/flute_types.h>
#include <flute/Logger.h>

#include <sys/select.h>

#include <map>

namespace flute {
namespace detail {

class SelectSelector : public Selector {
public:
    SelectSelector() : m_maxDescriptor(0), m_readSet(), m_writeSet(), m_dataMap() {}
    ~SelectSelector() = default;

    void addEvent(socket_type descriptor, int old, int events, void* data) override {
        auto temp = old | events;
        m_maxDescriptor = descriptor > m_maxDescriptor ? descriptor : m_maxDescriptor;
        if (descriptor >= m_readSet.size()) {
            m_readSet.resize(m_maxDescriptor + 1);
        }
        if (descriptor >= m_writeSet.size()) {
            m_writeSet.resize(m_maxDescriptor + 1);
        }
        if (events & SelectorEvent::EVENT_READ) {
            FD_SET(descriptor, m_readSet.data());
        }
        if (events & SelectorEvent::EVENT_WRITE) {
            FD_SET(descriptor, m_writeSet.data());
        }
        if (temp != SelectorEvent::EVENT_NONE) {
            m_dataMap[descriptor] = data;
        } else {
            m_dataMap.erase(descriptor);
        }
    }

    void removeEvent(socket_type descriptor, int old, int events, void* data) override {
        auto temp = old & (~events);
        m_maxDescriptor = descriptor > m_maxDescriptor ? descriptor : m_maxDescriptor;
        if (descriptor >= m_maxDescriptor) {
            m_readSet.resize(m_maxDescriptor + 1);
        }
        if (descriptor >= m_maxDescriptor) {
            m_writeSet.resize(m_maxDescriptor + 1);
        }
        if (events & SelectorEvent::EVENT_READ) {
            FD_CLR(descriptor, m_readSet.data());
        }
        if (events & SelectorEvent::EVENT_WRITE) {
            FD_CLR(descriptor, m_writeSet.data());
        }
        if (temp == SelectorEvent::EVENT_NONE) {
            m_dataMap.erase(descriptor);
        } else {
            m_dataMap[descriptor] = data;
        }
    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {
        int count = 0;
        std::vector<fd_set> readSet = m_readSet;
        std::vector<fd_set> writeSet = m_writeSet;
        if (timeout > 0) {
            struct timeval timeoutSpec;
            timeoutSpec.tv_sec = timeout / 1000;
            timeoutSpec.tv_usec = (timeout % 1000) * 1000;
            count = ::select(m_readSet.size(), readSet.data(), writeSet.data(), nullptr, &timeoutSpec);
        } else {
            count = ::select(m_readSet.size(), readSet.data(), writeSet.data(), nullptr, nullptr);
        }
        if (count == -1) {
            LOG_ERROR << "poll error " << errno << ":" << std::strerror(errno);
            return -1;
        }
        if (count > 0 && static_cast<std::size_t>(count) > events.size()) {
            events.resize(count);
        }
        auto index = 0;
        for (socket_type i = 0; i < m_maxDescriptor; ++i) {
            auto& e = events[index];
            auto it = m_dataMap.find(i);
            e.data = it->second;
            e.events = 0;
            if (FD_ISSET(i, m_readSet.data())) e.events |= SelectorEvent::EVENT_READ;
            if (FD_ISSET(i, m_writeSet.data())) e.events |= SelectorEvent::EVENT_WRITE;
            index += 1;
        }
        return count;
    }

private:
    socket_type m_maxDescriptor;
    std::vector<fd_set> m_readSet;
    std::vector<fd_set> m_writeSet;
    std::map<socket_type, void*> m_dataMap;
};

} // namespace detail
} // namespace flute


#endif // FLUTE_DETAIL_SELECT_SELECTOR_H