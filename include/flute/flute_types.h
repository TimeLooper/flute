//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_TYPES_H
#define FLUTE_TYPES_H

#include <flute/flute-config.h>

#include <functional>
#include <memory>

#if defined(_WIN32) || defined(WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <io.h>
#include <process.h>
#include <tchar.h>
#undef _WIN32_WINNT
/* For structs needed by GetAdaptersAddresses */
#define _WIN32_WINNT 0x0501
#include <iphlpapi.h>
#endif
#ifdef FLUTE_HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

namespace flute {

#if defined(WIN32) || defined(_WIN32)
typedef SOCKET socket_type;
#define SHUT_WR SD_SEND
#define SHUT_RD SD_RECEIVE
#else
typedef int socket_type;
#endif

const socket_type FLUTE_INVALID_SOCKET = static_cast<socket_type>(~0);

#ifdef WIN32
typedef int ssize_t;
#elif _WIN64
typedef long int ssize_t;
#else
typedef ::ssize_t ssize_t;
#endif

} // namespace flute

#endif // FLUTE_TYPES_H