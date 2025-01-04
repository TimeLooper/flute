#include <flute/AsyncIoService.h>

#ifdef _WIN32
#include <flute/detail/IocpService.h>
#endif // _WIN32

namespace flute {

AsyncIoService* AsyncIoService::createAsyncIoService(std::size_t asyncIoWorkThreadCount) {
#ifdef _WIN32
    return new detail::IocpService(asyncIoWorkThreadCount);
#endif // _WIN32
    return nullptr;
}

} // namespace flute