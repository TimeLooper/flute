#ifndef FLUTE_ASYNC_IO_SERVICE_H
#define FLUTE_ASYNC_IO_SERVICE_H

#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

namespace flute {

class AsyncIoService;

enum AsyncIoCode {
    IoCodeSuccess = 0,
    IoCodeFailed = 1,
    IoCodeEof = 2,
};

// do not new this object
struct AsyncIoContext {
    void* userData;
    socket_type socket;
    socket_type acceptSocket;
    SocketOpCode opCode;
    std::function<void(AsyncIoCode, flute::ssize_t, AsyncIoContext*)> ioCompleteCallback;
    AsyncIoContext()
        : userData(nullptr)
        , socket(FLUTE_INVALID_SOCKET)
        , acceptSocket(FLUTE_INVALID_SOCKET)
        , opCode(SocketOpCode::None)
        , ioCompleteCallback() {}
};

class AsyncIoService : private noncopyable {
public:
    virtual void dispatch() = 0;
    virtual void shutdown() = 0;
    virtual bool post(AsyncIoContext* context) = 0;
    virtual AsyncIoContext* createIoContext() = 0;
    virtual void destroyIoContext(AsyncIoContext* context) = 0;
    virtual void setIoContextBuffer(AsyncIoContext* context, iovec* vec, flute::ssize_t count) = 0;
    virtual void bindIoService(socket_type socket) = 0;
    static AsyncIoService* createAsyncIoService(std::size_t asyncIoWorkThreadCount);
    virtual ~AsyncIoService() = default;
};

} // namespace flute

#endif // FLUTE_ASYNC_IO_SERVICE_H
