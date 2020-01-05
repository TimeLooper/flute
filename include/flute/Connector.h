//
// Created by why on 2020/01/05.
//

#ifndef FLUTE_CONNECTOR_H
#define FLUTE_CONNECTOR_H

#include <flute/config.h>
#include <flute/noncopyable.h>

namespace flute {

class Connector : private noncopyable {
public:
    FLUTE_API_DECL Connector();
    FLUTE_API_DECL ~Connector();

private:

};

} // namespace flute


#endif // FLUTE_CONNECTOR_H