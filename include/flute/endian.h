//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_ENDIAN_H
#define FLUTE_ENDIAN_H

#include <flute/flute-config.h>

#ifdef FLUTE_HAVE_ENDIAN_H
#include <endian.h>
#endif
#ifdef FLUTE_HAVE_MACHINE_ENDIAN_H
#ifndef __FreeBSD__
#include <machine/endian.h>
#else
#include <sys/endian.h>
#endif
#endif
#ifdef _WIN32
#include <WinSock2.h>
#endif
#include <cstdint>

namespace flute {

inline std::uint64_t host2Network(std::uint64_t value) {
    return (((value) & 0xff00000000000000ull) >> 56)
		     | (((value) & 0x00ff000000000000ull) >> 40)
		     | (((value) & 0x0000ff0000000000ull) >> 24)
		     | (((value) & 0x000000ff00000000ull) >> 8)
		     | (((value) & 0x00000000ff000000ull) << 8)
		     | (((value) & 0x0000000000ff0000ull) << 24)
		     | (((value) & 0x000000000000ff00ull) << 40)
		     | (((value) & 0x00000000000000ffull) << 56);
}

inline std::int64_t host2Network(std::int64_t value) { return host2Network(static_cast<std::uint64_t>(value)); }

inline std::uint32_t host2Network(std::uint32_t value) {
    return ((((value) & 0xff000000) >> 24) | (((value) & 0x00ff0000) >>  8) | (((value) & 0x0000ff00) <<  8) | (((value) & 0x000000ff) << 24));
}

inline std::int32_t host2Network(std::int32_t value) { return host2Network(static_cast<std::uint32_t>(value)); }

inline std::uint16_t host2Network(std::uint16_t value) {
    return static_cast<std::uint16_t>((((value) >> 8) & 0xff) | (((value) & 0xff) << 8));
}

inline std::int16_t host2Network(std::int16_t value) { return host2Network(static_cast<std::uint16_t>(value)); }

inline std::uint64_t network2Host(std::uint64_t value) {
    return (((value) & 0xff00000000000000ull) >> 56)
		     | (((value) & 0x00ff000000000000ull) >> 40)
		     | (((value) & 0x0000ff0000000000ull) >> 24)
		     | (((value) & 0x000000ff00000000ull) >> 8)
		     | (((value) & 0x00000000ff000000ull) << 8)
		     | (((value) & 0x0000000000ff0000ull) << 24)
		     | (((value) & 0x000000000000ff00ull) << 40)
		     | (((value) & 0x00000000000000ffull) << 56);
}

inline std::int64_t network2Host(std::int64_t value) { return network2Host(static_cast<std::uint64_t>(value)); }

inline std::uint32_t network2Host(std::uint32_t value) {
    return ((((value) & 0xff000000) >> 24) | (((value) & 0x00ff0000) >>  8) | (((value) & 0x0000ff00) <<  8) | (((value) & 0x000000ff) << 24));
}

inline std::int32_t network2Host(std::int32_t value) { return network2Host(static_cast<std::uint32_t>(value)); }

inline std::uint16_t network2Host(std::uint16_t value) {
    return static_cast<std::uint16_t>((((value) >> 8) & 0xff) | (((value) & 0xff) << 8));
}

inline std::int16_t network2Host(std::int16_t value) { return network2Host(static_cast<std::uint16_t>(value)); }

} // namespace flute

#endif // FLUTE_ENDIAN_H