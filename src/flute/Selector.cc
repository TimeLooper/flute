//
// Created by why on 2019/12/29.
//

#include <flute/Selector.h>

#include <flute/flute-config.h>

#define INIT_EVENT_SIZE 32
#define MAX_EVENT_SIZE 4096

#ifdef FLUTE_HAVE_EPOLL
#include <flute/detail/EPollSelector.h>
#endif

namespace flute {

Selector* Selector::createSelector() {
#ifdef FLUTE_HAVE_EPOLL
    return new detail::EPollSelector();
#endif
}

} // namespace flute
