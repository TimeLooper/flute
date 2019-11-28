/*************************************************************************
 *
 * File Name:  Reactor.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include <flute/Reactor.h>
#include <flute/flute-config.h>

#ifdef FLUTE_HAVE_EPOLL
#include <flute/impl/EpollReactor.h>
#endif

namespace flute {

Reactor* createReactor() {
#ifdef FLUTE_HAVE_EPOLL
    return new impl::EpollReactor();
#endif
    return nullptr;
}

} // namespace flute