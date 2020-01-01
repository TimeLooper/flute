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

#ifdef _WIN32
struct flute_fd_set {
    unsigned int fd_count;             /* how many are SET? */
    socket_type  fd_array[0];          /* an array of SOCKETs */
};
#else
// typedef fd_set flute_fd_set;
struct flute_fd_set {
    std::uint8_t fds_bits[1];
};
#endif

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
    flute_fd_set *m_set;

    void checkSize(socket_type descriptor);

    static const int INIT_SET_SIZE = FD_SETSIZE;
};

#ifdef _WIN32

inline DescriptorSet::DescriptorSet() : m_maxDescriptor(0), m_setSize(INIT_SET_SIZE), m_set(static_cast<flute_fd_set *>(std::malloc(sizeof(flute_fd_set) + INIT_SET_SIZE * sizeof(socket_type)))) {
    std::memset(m_set, 0, sizeof(flute_fd_set) + INIT_SET_SIZE * sizeof(socket_type));
}

inline DescriptorSet::DescriptorSet(const DescriptorSet& rhs) : DescriptorSet() {
    if (m_setSize < rhs.m_setSize) {
        m_set = static_cast<flute_fd_set *>(std::realloc(m_set, sizeof(flute_fd_set) + rhs.m_setSize * sizeof(socket_type)));
    }
    m_setSize = rhs.m_setSize;
    std::memcpy(m_set, rhs.m_set, sizeof(flute_fd_set) + rhs.m_setSize * sizeof(socket_type));
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
    if (this == &rhs) {
        return *this;
    }
    if (m_setSize < rhs.m_setSize) {
        m_set = static_cast<flute_fd_set *>(std::realloc(m_set, sizeof(flute_fd_set) + rhs.m_setSize * sizeof(socket_type)));
    }
    m_setSize = rhs.m_setSize;
    std::memcpy(m_set, rhs.m_set, sizeof(flute_fd_set) + rhs.m_setSize * sizeof(socket_type));
    m_maxDescriptor = rhs.m_maxDescriptor;
    return *this;
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& rhs) {
    if (this == &rhs) {
        return *this;
    }
    std::swap(m_set, rhs.m_set);
    std::swap(m_setSize, rhs.m_setSize);
    std::swap(m_maxDescriptor, rhs.m_maxDescriptor);
    return *this;
}

inline bool DescriptorSet::containes(socket_type descriptor) {
    unsigned int i;
    for (i = 0; i < m_set->fd_count; i++) {
        if (m_set->fd_array[i] == descriptor) {
            break;
        }
    }
    return i < m_set->fd_count;
}

inline void DescriptorSet::add(socket_type descriptor) {
    unsigned int i;
    for (i = 0; i < m_set->fd_count; i++) {
        if (m_set->fd_array[i] == descriptor) {
            break;
        }
    }
    if (i == m_set->fd_count) {
        if (m_set->fd_count >= m_setSize) {
            m_set = static_cast<flute_fd_set *>(std::realloc(m_set, sizeof(flute_fd_set) + (m_setSize << 1) * sizeof(socket_type)));
            m_setSize <<= 1;
        }
        m_set->fd_array[i] = descriptor;
        m_set->fd_count += 1;
    }
}

inline void DescriptorSet::remove(socket_type descriptor) {
    unsigned int i;
    for (i = 0; i < m_set->fd_count; i++) {
        if (m_set->fd_array[i] == descriptor) {
            break;
        }
    }
    if (i < m_set->fd_count) {
        std::memmove(m_set->fd_array + i, m_set->fd_array + i + 1, m_set->fd_count - 1 - i);
        m_set->fd_count -= 1;
    }
}

inline fd_set* DescriptorSet::getRawSet() {
    return reinterpret_cast<fd_set*>(m_set);
}

#else

inline DescriptorSet::DescriptorSet() : m_maxDescriptor(0), m_setSize(INIT_SET_SIZE), m_set(static_cast<flute_fd_set *>(std::malloc(m_setSize))) {
    std::memset(m_set, 0, sizeof(m_setSize));
}

inline DescriptorSet::DescriptorSet(const DescriptorSet& rhs) : DescriptorSet() {
    if (m_setSize < rhs.m_setSize) {
        m_set = static_cast<flute_fd_set *>(std::realloc(m_set, rhs.m_setSize));
    }
    std::memcpy(m_set->fds_bits, rhs.m_set->fds_bits, rhs.m_setSize);
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
    if (this == &rhs) {
        return *this;
    }
    if (m_setSize < rhs.m_setSize) {
        m_set = static_cast<flute_fd_set *>(std::realloc(m_set, rhs.m_setSize));
    }
    std::memcpy(m_set->fds_bits, rhs.m_set->fds_bits, rhs.m_setSize);
    m_setSize = rhs.m_setSize;
    m_maxDescriptor = rhs.m_maxDescriptor;
    return *this;
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& rhs) {
    if (this == &rhs) {
        return *this;
    }
    std::swap(m_set, rhs.m_set);
    std::swap(m_setSize, rhs.m_setSize);
    std::swap(m_maxDescriptor, rhs.m_maxDescriptor);
    return *this;
}

inline bool DescriptorSet::containes(socket_type descriptor) {
    // auto index = static_cast<std::size_t>(descriptor / 8);
    auto index = static_cast<std::size_t>(descriptor >> 3);
    if (index >= m_setSize) {
        return false;
    }
    return m_set->fds_bits[index] & (std::uint8_t (0x1)) << (descriptor & 8);
}

inline void DescriptorSet::add(socket_type descriptor) {
    // auto index = static_cast<std::size_t>(descriptor / 8);
    auto index = static_cast<std::size_t>(descriptor >> 3);
    if (index >= m_setSize) {
        m_set = static_cast<flute_fd_set *>(std::realloc(m_set, index + 1));
        m_setSize = index + 1;
    }
    m_set->fds_bits[index] |= (std::uint8_t (0x1)) << (descriptor & 8);
}

inline void DescriptorSet::remove(socket_type descriptor) {
    // auto index = static_cast<std::size_t>(descriptor / 8);
    auto index = static_cast<std::size_t>(descriptor >> 3);
    if (index >= m_setSize) {
        return;
    }
    m_set->fds_bits[index] &= ~((std::uint8_t (0x1)) << (descriptor & 8));
}

inline fd_set* DescriptorSet::getRawSet() {
    return reinterpret_cast<fd_set*>(m_set);
}

#endif

} // namespace detail
} // namespace flute

#endif // FLUTE_DETAIL_DESCRIPTOR_SET_H