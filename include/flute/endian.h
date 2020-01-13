//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_ENDIAN_H
#define FLUTE_ENDIAN_H

#include <flute/flute-config.h>

#ifdef FLUTE_HAVE_ENDIAN_H
#include <endian.h>
#endif
#ifdef FLUTE_HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif
#ifdef _WIN32
#include <WinSock2.h>
#endif
#include <cstdint>

namespace flute {

inline std::uint64_t host2Network(std::uint64_t value) {
#if defined(FLUTE_HAVE_ENDIAN_H) || defined(FLUTE_HAVE_SYS_ENDIAN_H)
    return htobe64(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(_WIN32)
    return htonll(value);
#endif
}

inline std::int64_t host2Network(std::int64_t value) { return host2Network(static_cast<std::uint64_t>(value)); }

inline std::uint32_t host2Network(std::uint32_t value) {
#if defined(FLUTE_HAVE_ENDIAN_H) || defined(FLUTE_HAVE_SYS_ENDIAN_H)
    return htobe32(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(_WIN32)
    return htonl(value);
#endif
}

inline std::int32_t host2Network(std::int32_t value) { return host2Network(static_cast<std::uint32_t>(value)); }

inline std::uint16_t host2Network(std::uint16_t value) {
#if defined(FLUTE_HAVE_ENDIAN_H) || defined(FLUTE_HAVE_SYS_ENDIAN_H)
    return htobe16(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(_WIN32)
    return htons(value);
#endif
}

inline std::int16_t host2Network(std::int16_t value) { return host2Network(static_cast<std::uint16_t>(value)); }

inline std::uint64_t network2Host(std::uint64_t value) {
#if defined(FLUTE_HAVE_ENDIAN_H) || defined(FLUTE_HAVE_SYS_ENDIAN_H)
    return be64toh(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(_WIN32)
    return ntohll(value);
#endif
}

inline std::int64_t network2Host(std::int64_t value) { return network2Host(static_cast<std::uint64_t>(value)); }

inline std::uint32_t network2Host(std::uint32_t value) {
#if defined(FLUTE_HAVE_ENDIAN_H) || defined(FLUTE_HAVE_SYS_ENDIAN_H)
    return be32toh(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(_WIN32)
    return ntohl(value);
#endif
}

inline std::int32_t network2Host(std::int32_t value) { return network2Host(static_cast<std::uint32_t>(value)); }

inline std::uint16_t network2Host(std::uint16_t value) {
#if defined(FLUTE_HAVE_ENDIAN_H) || defined(FLUTE_HAVE_SYS_ENDIAN_H)
    return be16toh(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(_WIN32)
    return ntohs(value);
#endif
}

inline std::int16_t network2Host(std::int16_t value) { return network2Host(static_cast<std::uint16_t>(value)); }

} // namespace flute

#endif // FLUTE_ENDIAN_H