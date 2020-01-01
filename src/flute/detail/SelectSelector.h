//
// Created by why on 2019/12/31.
//

#ifndef FLUTE_DETAIL_SELECT_SELECTOR_H
#define FLUTE_DETAIL_SELECT_SELECTOR_H

#include <flute/flute-config.h>
#include <flute/Selector.h>
#include <flute/flute_types.h>
#include <flute/Logger.h>

#include <flute/detail/DescriptorSet.h>

#ifdef FLUTE_HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

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
        if (events & SelectorEvent::EVENT_READ) {
            m_readSet.add(descriptor);
        }
        if (events & SelectorEvent::EVENT_WRITE) {
            m_writeSet.add(descriptor);
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
        if (events & SelectorEvent::EVENT_READ) {
            m_readSet.remove(descriptor);
        }
        if (events & SelectorEvent::EVENT_WRITE) {
            m_writeSet.remove(descriptor);
        }
        if (temp == SelectorEvent::EVENT_NONE) {
            m_dataMap.erase(descriptor);
        } else {
            m_dataMap[descriptor] = data;
        }
    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {
        int count = 0;
        DescriptorSet readSet = m_readSet;
        DescriptorSet writeSet = m_writeSet;
        if (timeout > 0) {
            struct timeval timeoutSpec;
            timeoutSpec.tv_sec = timeout / 1000;
            timeoutSpec.tv_usec = (timeout % 1000) * 1000;
            count = ::select(m_maxDescriptor + 1, readSet.getRawSet(), writeSet.getRawSet(), nullptr, &timeoutSpec);
        } else {
            count = ::select(m_maxDescriptor + 1, readSet.getRawSet(), writeSet.getRawSet(), nullptr, nullptr);
        }
        if (count == -1) {
            LOG_ERROR << "select error " << errno << ":" << std::strerror(errno);
            return -1;
        }
        if (count > 0 && static_cast<std::size_t>(count) > events.size()) {
            events.resize(count);
        }
        auto index = 0;
        for (socket_type i = 0; i <= m_maxDescriptor; ++i) {
            auto& e = events[index];
            auto events = 0;
            if (readSet.containes(i)) events |= SelectorEvent::EVENT_READ;
            if (writeSet.containes(i)) events |= SelectorEvent::EVENT_WRITE;
            if (events) {
                auto it = m_dataMap.find(i);
                e.data = it->second;
                e.events = events;
                index += 1;
            }
        }
        return index;
    }

private:
    socket_type m_maxDescriptor;
    DescriptorSet m_readSet;
    DescriptorSet m_writeSet;
    std::map<socket_type, void*> m_dataMap;
};

} // namespace detail
} // namespace flute


#endif // FLUTE_DETAIL_SELECT_SELECTOR_H