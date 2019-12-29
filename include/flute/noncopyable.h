//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_NONCOPYABLE_H
#define FLUTE_NONCOPYABLE_H

namespace flute {

class noncopyable {
public:
    noncopyable() = default;
    ~noncopyable() = default;

    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
};

} // namespace flute

#endif // FLUTE_NONCOPYABLE_H