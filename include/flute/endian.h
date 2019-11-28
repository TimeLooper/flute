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
#include <cstdint>

namespace flute {

inline std::uint64_t host2Network(std::uint64_t host64) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return htobe64(host64);
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
    return htonll(host64);
#endif
}

inline std::uint32_t host2Network(std::uint32_t host32) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return htobe32(host32);
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
    return htonl(host32);
#endif
}

inline std::uint16_t host2Network(std::uint16_t host16) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return htobe16(host16);
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
    return htons(host16);
#endif
}

inline std::uint64_t network2Host(std::uint64_t net64) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return be64toh(net64);
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
    return ntohll(net64);
#endif
}

inline std::uint32_t network2Host(std::uint32_t net32) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return be32toh(net32);
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
    return ntohl(net32);
#endif
}

inline std::uint16_t network2Host(std::uint16_t net16) {
#ifdef FLUTE_HAVE_ENDIAN_H
    return be16toh(net16);
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
    return ntohs(net16);
#endif
}

} // namespace flute