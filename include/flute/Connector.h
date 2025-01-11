//
// Created by why on 2020/01/05.
//

#ifndef FLUTE_CONNECTOR_H
#define FLUTE_CONNECTOR_H

#include <flute/AsyncIoService.h>
#include <flute/InetAddress.h>
#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <atomic>
#include <future>
#include <memory>

namespace flute {

class EventLoop;
class Channel;

class Connector : private noncopyable, public std::enable_shared_from_this<Connector> {
public:
    FLUTE_API_DECL Connector(EventLoop* loop, const InetAddress& address);
    FLUTE_API_DECL Connector(EventLoop* loop, InetAddress&& address);
    FLUTE_API_DECL ~Connector();

    FLUTE_API_DECL void start();
    FLUTE_API_DECL void restart();
    FLUTE_API_DECL void stop();

    inline void setConnectCallback(ConnectCallback&& callback) { m_connectCallback = std::move(callback); }
    inline void setConnectCallback(const ConnectCallback& callback) { m_connectCallback = callback; }
    inline const InetAddress& getServerAddress() const { return m_serverAddress; }

private:
    enum class ConnectorState { DISCONNECTED, CONNECTING, CONNECTED };
    int m_retryDelay;
    std::uint64_t m_retry_timer_id;
    EventLoop* m_loop;
    Channel* m_channel;
    AsyncIoContext* m_ioContext;
    InetAddress m_serverAddress;
    std::atomic<ConnectorState> m_state;
    std::atomic<bool> m_isConnect;
    ConnectCallback m_connectCallback;
    std::promise<void> m_stop_promise;

    // 30 * 1000
    static const int MAX_RETRY_DELAY;
    // 500
    static const int DEFALUT_RETRY_DELAY;

    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(socket_type descriptor);
    void retry(socket_type descriptor);
    void handleWrite();
    void resetChannel();
    socket_type removeAndResetChannel();
    void handleAsyncConnect(AsyncIoCode code, ssize_t bytes, AsyncIoContext* ioContext);
    void handleAsyncConnectInLoop(AsyncIoCode code, ssize_t bytes, AsyncIoContext* ioContext);
};

} // namespace flute

#endif // FLUTE_CONNECTOR_H
