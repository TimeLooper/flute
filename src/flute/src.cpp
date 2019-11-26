/*************************************************************************
 *
 * File Name:  src.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/27 00:30:01
 *
 *************************************************************************/

#define FLUTE_SOURCES

#include <flute/config.hpp>

#ifdef FLUTE_HEADER_ONLY
#error "Do not build dynamic library with FLUTE_HEADER_ONLY define"
#endif

#include <flute/impl/epoll_event_loop.inl>