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
#include <cstdint>

namespace flute {

inline std::uint64_t host2Network(std::uint64_t host64) {
    return htobe64(host64);
}

inline std::uint32_t host2Network(std::uint32_t host32) {
    return htobe32(host32);
}

inline std::uint16_t host2Network(std::uint16_t host16) {
    return htobe16(host16);
}

inline std::uint64_t network2Host(std::uint64_t net64) {
    return be64toh(net64);
}

inline std::uint32_t network2Host(std::uint32_t net32) {
    return be32toh(net32);
}

inline std::uint16_t network2Host(std::uint16_t net16) {
    return be16toh(net16);
}

} // namespace flute