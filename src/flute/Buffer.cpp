/*************************************************************************
 *
 * File Name:  Buffer.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/08 21:31:01
 *
 *************************************************************************/

#include <flute/Buffer.h>

namespace flute {

struct BufferChain {
    std::size_t prependIndex;
    std::size_t writeIndex;
    std::size_t capacity;
    std::uint8_t* buffer;
};

} // namespace flute
