//
// Created by why on 2020/01/01.
//

#ifndef FLUTE_DETAIL_DESCRIPTOR_SET_H
#define FLUTE_DETAIL_DESCRIPTOR_SET_H

#include <flute/flute-config.h>
#include <flute/flute_types.h>

#include <cstdlib>

#ifdef FLUTE_HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

namespace flute {
namespace detail {

struct DescriptorSet {
public:
    DescriptorSet();
    DescriptorSet(const DescriptorSet& rhs);
    DescriptorSet(DescriptorSet&& rhs);
    ~DescriptorSet();

    DescriptorSet& operator=(const DescriptorSet& rhs);
    DescriptorSet& operator=(DescriptorSet&& rhs);

    bool containes(socket_type descriptor);

    void add(socket_type descriptor);

    void remove(socket_type descriptor);

    fd_set* getRawSet();

private:
    socket_type m_maxDescriptor;
    std::size_t m_setSize;
    fd_set *m_set;

    void checkSize(socket_type descriptor);
};

inline DescriptorSet::DescriptorSet() : m_maxDescriptor(0), m_setSize(sizeof(fd_set)), m_set(static_cast<fd_set *>(std::malloc(sizeof(fd_set)))) {
    std::memset(m_set, 0, sizeof(fd_set));
}

inline DescriptorSet::DescriptorSet(const DescriptorSet& rhs) : DescriptorSet() {
    *m_set = *rhs.m_set;
    m_setSize = rhs.m_setSize;
    m_maxDescriptor = rhs.m_maxDescriptor;
}

inline DescriptorSet::DescriptorSet(DescriptorSet&& rhs) : DescriptorSet() {
    std::swap(m_set, rhs.m_set);
    std::swap(m_setSize, rhs.m_setSize);
    std::swap(m_maxDescriptor, rhs.m_maxDescriptor);
}

inline DescriptorSet::~DescriptorSet() {
    std::free(m_set);
}

DescriptorSet& DescriptorSet::operator=(const DescriptorSet& rhs) {
    *m_set = *rhs.m_set;
    m_setSize = rhs.m_setSize;
    m_maxDescriptor = rhs.m_maxDescriptor;
    return *this;
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& rhs) {
    std::swap(m_set, rhs.m_set);
    std::swap(m_setSize, rhs.m_setSize);
    std::swap(m_maxDescriptor, rhs.m_maxDescriptor);
    return *this;
}

inline bool DescriptorSet::containes(socket_type descriptor) {
    return FD_ISSET(descriptor, m_set);
}

inline void DescriptorSet::add(socket_type descriptor) {
    FD_SET(descriptor, m_set);
}

inline void DescriptorSet::remove(socket_type descriptor) {
    FD_CLR(descriptor, m_set);
}

inline fd_set* DescriptorSet::getRawSet() {
    return m_set;
}

} // namespace detail
} // namespace flute

#endif // FLUTE_DETAIL_DESCRIPTOR_SET_H