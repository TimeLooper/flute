//
// Created by why on 2019/12/29.
//

#include "Reactor.h"

#include <flute/flute-config.h>

#ifdef FLUTE_HAVE_EPOLL
#include <flute/detail/EPollReactor.h>
#endif

namespace flute {

Reactor* Reactor::createReactor() {
#ifdef FLUTE_HAVE_EPOLL
    return new detail::EPollReactor();
#endif
}

} // namespace flute
