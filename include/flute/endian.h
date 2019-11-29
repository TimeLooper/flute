/*************************************************************************
 *
 * File Name:  endian.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include <flute/flute-config.h>

#ifdef FLUTE_HAVE_ENDIAN_H
#include <endian.h>
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif
#if defined(WIN32) || defined(_WIN32)
#include <WinSock2.h>
#endif
#include <cstdint>

namespace flute {

inline std::uint64_t host2Network(std::uint64_t value) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return htobe64(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(WIN32) || defined(_WIN32)
    return htonll(value);
#endif
}

inline std::uint32_t host2Network(std::uint32_t value) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return htobe32(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(WIN32) || defined(_WIN32)
    return htonl(value);
#endif
}

inline std::uint16_t host2Network(std::uint16_t value) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return htobe16(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(WIN32) || defined(_WIN32)
    return htons(value);
#endif
}

inline std::uint64_t network2Host(std::uint64_t value) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return be64toh(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(WIN32) || defined(_WIN32)
    return ntohll(value);
#endif
}

inline std::uint32_t network2Host(std::uint32_t value) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return be32toh(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(WIN32) || defined(_WIN32)
    return ntohl(value);
#endif
}

inline std::uint16_t network2Host(std::uint16_t value) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return be16toh(value);
#endif
#if defined(FLUTE_HAVE_MACHINE_ENDIAN_H) || defined(WIN32) || defined(_WIN32)
    return ntohs(value);
#endif
}

} // namespace flute