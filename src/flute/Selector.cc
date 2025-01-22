//
// Created by why on 2019/12/29.
//

#include <flute/Selector.h>
#include <flute/flute-config.h>

#define INIT_EVENT_SIZE 32
#define MAX_EVENT_SIZE 4096

#ifdef FLUTE_HAVE_EPOLL
#include <flute/detail/EpollSelector.h>
#endif
#ifdef FLUTE_HAVE_KQUEUE
#include <flute/detail/KqueueSelector.h>
#endif
#if defined(FLUTE_HAVE_POLL) || defined(_WIN32)
#include <flute/detail/PollSelector.h>
#endif
#if defined(FLUTE_HAVE_SELECT) || defined(_WIN32)
#include <flute/detail/SelectSelector.h>
#endif

namespace flute {

SelectorEvent::SelectorEvent() : events(0), data(nullptr) {}

Selector* Selector::createSelector() {
#ifdef FLUTE_HAVE_EPOLL
    return new detail::EpollSelector();
#endif
#ifdef FLUTE_HAVE_KQUEUE
    return new detail::KqueueSelector();
#endif
#if defined(FLUTE_HAVE_POLL) || defined(_WIN32)
    return new detail::PollSelector();
#endif
#if defined(FLUTE_HAVE_SELECT) || defined(_WIN32)
    return new detail::SelectSelector();
#endif
}

} // namespace flute
