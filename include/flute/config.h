/*************************************************************************
 *
 * File Name:  config.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/27
 *
 *************************************************************************/

#pragma once

#ifdef flute_shared_EXPORTS
#if defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#define FLUTE_API_DECL __global
#elif defined(__GNUC__)
#define FLUTE_API_DECL __attribute__((visibility("default")))
#elif defined(_MSC_VER)
#if defined(FLUTE_INSIDE_LIBRARY)
#define FLUTE_API_DECL __declspec(dllexport)
#else
#define FLUTE_API_DECL __declspec(dllimport)
#endif
#endif
#else
#define FLUTE_API_DECL
#endif