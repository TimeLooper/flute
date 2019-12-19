/*************************************************************************
 *
 * File Name:  Connector.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/19 23:38:47
 *
 *************************************************************************/

#pragma once

#include <flute/config.h>
#include <flute/flute_types.h>
#include <flute/InetAddress.h>
#include <flute/noncopyable.h>

#include <atomic>
#include <functional>
#include <memory>

namespace flute {

class EventLoopGroup;
class Channel;
class EventLoop;

class Connector : private noncopyable {
public:
    FLUTE_API_DECL Connector(EventLoopGroup* loop, const InetAddress& address);
    FLUTE_API_DECL Connector(EventLoopGroup* loop, InetAddress&& address);
    FLUTE_API_DECL ~Connector();

    FLUTE_API_DECL void start();
    FLUTE_API_DECL void stop();
    FLUTE_API_DECL void restart();

    inline void setOnConnectedCallback(const ConnectCallback& callback) {
        m_connectedCallback = callback;
    }
    inline void setOnConnectedCallback(ConnectCallback&& callback) {
        m_connectedCallback = std::move(callback);
    }
    inline const InetAddress& getRemoteAddress() const {
        return m_remoteAddress;
    }

private:
    enum ConnectorState { DISCONNECTED, CONNECTING, CONNECTED };
    static const int MAX_RETRY_DELAY_TIME;
    static const int DEFAULT_RETRY_DELAY_TIME;
    int m_retryDelay;
    EventLoop* m_loop;
    InetAddress m_remoteAddress;
    std::atomic<bool> m_connect;
    std::atomic<ConnectorState> m_state;
    std::unique_ptr<Channel> m_channel;
    ConnectCallback m_connectedCallback;

    void startInLoop();
    void stopInLoop();
    void connect();
    void connecting(socket_type descriptor);
    void retry(socket_type descriptor);
    void handleWrite();
    // void handleError();
    socket_type removeAndResetChannel();
    void resetChannel();
    inline void setState(ConnectorState state) {
        m_state = state;
    }
};

} // namespace flute