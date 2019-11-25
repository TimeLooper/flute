/*************************************************************************
 *
 * File Name:  socket_types.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26 02:15:33
 *
 *************************************************************************/

#pragma once

#include <flute/flute-config.hpp>

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
#else
typedef int socket_type;
#endif

const socket_type INVALID_SOCKET = static_cast<socket_type>(~0);

} // namespace flute