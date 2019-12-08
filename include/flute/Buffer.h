/*************************************************************************
 *
 * File Name:  Buffer.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/08 21:30:50
 *
 *************************************************************************/

#pragma once

#include <list>

namespace flute {

struct BufferChain;

class Buffer {
public:
    Buffer();
    ~Buffer();

    void append
private:
    std::list<BufferChain *> m_chains;
};

} // namespace flute