/*************************************************************************
 *
 * File Name:  config.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/27 00:05:35
 *
 *************************************************************************/

#pragma once

#ifndef flute_shared_EXPORTS
# ifndef FLUTE_HEADER_ONLY
#  define FLUTE_HEADER_ONLY
# endif
#endif

#ifdef FLUTE_HEADER_ONLY
# define FLUTE_API_DECL inline
#else
# if defined (__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#  define FLUTE_API_DECL __global
# elif defined(__GNUC__)
#  define FLUTE_API_DECL __attribute__((visibility("default")))
# elif defined(_MSC_VER)
#  if defined(FLUTE_SOURCES)
#    define FLUTE_API_DECL __declspec(dllexport)
#  else
#    define FLUTE_API_DECL __declspec(dllimport)
#  endif
# endif
#endif