//
// Created by why on 2024/12/29.
//

#ifndef FLUTE_DETAIL_IOCP_SERVICE_H
#define FLUTE_DETAIL_IOCP_SERVICE_H

#include <flute/flute_types.h>
#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/ThreadPool.h>
#include <flute/AsyncIoService.h>
#include <flute/socket_ops.h>
#include <flute/Logger.h>

#include <MSWSock.h>

namespace flute {
namespace detail {

#define NOTIFICATION_KEY ((ULONG_PTR)-1)

struct IocpOverlapped {
    OVERLAPPED overlapped;
    AsyncIoContext context;
    WSABUF* wsaBuf;
    DWORD bufferCount;
    IocpOverlapped() : overlapped(), context(), wsaBuf(nullptr), bufferCount(0) {
        std::memset(&overlapped, 0, sizeof(OVERLAPPED));
    }
};

static void *
getExtensionFunction(SOCKET s, const GUID *which_fn)
{
	void *ptr = NULL;
	DWORD bytes=0;
	WSAIoctl(s, SIO_GET_EXTENSION_FUNCTION_POINTER,
	    (GUID*)which_fn, sizeof(*which_fn),
	    &ptr, sizeof(ptr),
	    &bytes, NULL, NULL);
	return ptr;
}

class IocpService : public AsyncIoService {
public:
    IocpService(std::size_t asyncIoWorkThreadCount)
        : m_asyncIoWorkThreadCount(static_cast<int>(asyncIoWorkThreadCount))
        , m_iocp(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, static_cast<DWORD>(asyncIoWorkThreadCount)))
        , m_threadPool() {
        m_threadPool.start(asyncIoWorkThreadCount);
        for (int i = 0; i < asyncIoWorkThreadCount; ++i) {
            m_threadPool.execute([this] { dispatch(); });
        }
        if (s_acceptEx == nullptr || s_connectEx == nullptr) {
            auto socket = flute::socket(AF_INET, SOCK_STREAM, 0);
            const GUID guidAcceptEx = WSAID_ACCEPTEX;
            const GUID guidConnectEx = WSAID_CONNECTEX;
            s_connectEx = reinterpret_cast<LPFN_CONNECTEX>(getExtensionFunction(socket, &guidConnectEx));
            s_acceptEx = reinterpret_cast<LPFN_ACCEPTEX>(getExtensionFunction(socket, &guidAcceptEx));
            flute::closeSocket(socket);
        }
    }
    ~IocpService() {
        m_threadPool.shutdown();
        ::CloseHandle(m_iocp);
    }
    void dispatch() override {
        while (true) {
            DWORD bytes = 0;
            ULONG_PTR key = 0;
            LPOVERLAPPED overlapped = nullptr;
            BOOL ok = ::GetQueuedCompletionStatus(m_iocp, &bytes, &key, &overlapped, INFINITE);
            if (key != NOTIFICATION_KEY && overlapped) {
                auto iocpOverlapped =
                    reinterpret_cast<IocpOverlapped *>(CONTAINING_RECORD(overlapped, IocpOverlapped, overlapped));
                if (iocpOverlapped->context.opCode == SocketOpCode::Accept) {
                    setsockopt(iocpOverlapped->context.acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                               (char *)&iocpOverlapped->context.socket, sizeof(iocpOverlapped->context.socket));
                } else if (iocpOverlapped->context.opCode == SocketOpCode::Connect) {
                    setsockopt(iocpOverlapped->context.socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, 0);
                }
                if (ok && bytes) {
                    iocpOverlapped->context.ioCompleteCallback(AsyncIoCode::IoCodeSuccess, bytes, &iocpOverlapped->context);
                } else if (!ok) {
                    iocpOverlapped->context.ioCompleteCallback(AsyncIoCode::IoCodeFailed, 0,  &iocpOverlapped->context);
                } else if (!bytes && iocpOverlapped->context.opCode == SocketOpCode::Accept) {
                    iocpOverlapped->context.ioCompleteCallback(AsyncIoCode::IoCodeSuccess, 0,  &iocpOverlapped->context);
                } else {
                    iocpOverlapped->context.ioCompleteCallback(AsyncIoCode::IoCodeEof, 0,  &iocpOverlapped->context);
                }
            } else if (!overlapped) {
                break;
            }
        }
    }
    void shutdown() override {
        for (auto i = 0; i < m_asyncIoWorkThreadCount; ++i) {
            ::PostQueuedCompletionStatus(m_iocp, 0, NOTIFICATION_KEY, nullptr);
        }
        m_threadPool.shutdown();
    }
    bool post(AsyncIoContext* context) override {
        auto iocpOverlapped = reinterpret_cast<IocpOverlapped *>(CONTAINING_RECORD(context, IocpOverlapped, context));
        if (context->opCode == SocketOpCode::Read) {
            DWORD bytesRead;
	        DWORD flags = 0;
            if (::WSARecv(context->socket, iocpOverlapped->wsaBuf, iocpOverlapped->bufferCount, &bytesRead, &flags, &iocpOverlapped->overlapped, nullptr)) {
                auto error = WSAGetLastError();
                if (error != WSA_IO_PENDING) {
                    LOG_ERROR << "WSARecv failed: " << error << " " << formatErrorString(error);
                    return false;
                }
            }
        } else if (context->opCode == SocketOpCode::Write) {
            DWORD bytesSent = 0;
            if (::WSASend(context->socket, iocpOverlapped->wsaBuf, iocpOverlapped->bufferCount, &bytesSent, 0, &iocpOverlapped->overlapped, nullptr)) {
                auto error = WSAGetLastError();
                if (error!= WSA_IO_PENDING) {
                    LOG_ERROR << "WSASend failed: " << error << " " << formatErrorString(error);
                    return false;
                }
            }
        } else if (context->opCode == SocketOpCode::Connect) {
            sockaddr_storage ss;
            std::memset(&ss, 0, sizeof(ss));
            sockaddr *sa = reinterpret_cast<struct sockaddr*>(context->userData);
            auto size = 0;
            if (sa->sa_family == AF_INET) {
                struct sockaddr_in *sin = (struct sockaddr_in *)&ss;
                sin->sin_family = AF_INET;
                sin->sin_addr.s_addr = INADDR_ANY;
                size = sizeof(sockaddr_in);
            } else if (sa->sa_family == AF_INET6) {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)&ss;
                sin6->sin6_family = AF_INET6;
                sin6->sin6_addr = in6addr_any;
                size = sizeof(sockaddr_in6);
            } else {
                /* Well, the user will have to bind() */
                return false;
            }
            if (bind(context->socket, (struct sockaddr *)&ss, sizeof(ss)) < 0 &&
                WSAGetLastError() != WSAEINVAL)
                return false;
            auto rc = s_connectEx(context->socket, sa, size, nullptr, 0, nullptr, &iocpOverlapped->overlapped);
            auto error = WSAGetLastError();
            if (!(rc || error == WSA_IO_PENDING)) {
                LOG_ERROR << "ConnectEx failed: " << error << " " << formatErrorString(error);
                return false;
            }
        } else if (context->opCode == SocketOpCode::Accept) {
            DWORD pending = 0;
            if (s_acceptEx(context->socket, context->acceptSocket, iocpOverlapped->wsaBuf->buf, 0, iocpOverlapped->wsaBuf->len / 2, iocpOverlapped->wsaBuf->len / 2, &pending, &iocpOverlapped->overlapped)) {
                // Immediate success
                setsockopt(context->acceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&context->socket, sizeof(context->socket));
                context->ioCompleteCallback(AsyncIoCode::IoCodeSuccess, 0, context);
            } else {
                auto error = WSAGetLastError();
                if (error != WSA_IO_PENDING) {
                    LOG_ERROR << "AcceptEx failed: " << error << " " << formatErrorString(error);
                    return false;
                }
            }
        }
        return true;
    }
    AsyncIoContext* createIoContext() override {
        auto overlapped = new IocpOverlapped();
        return &overlapped->context;
    }
    void destroyIoContext(AsyncIoContext* context) override {
        auto iocpOverlapped = reinterpret_cast<IocpOverlapped *>(CONTAINING_RECORD(context, IocpOverlapped, context));
        delete iocpOverlapped;
    }
    void setIoContextBuffer(AsyncIoContext* context, iovec* vec, flute::ssize_t count) override {
        auto iocpOverlapped = reinterpret_cast<IocpOverlapped *>(CONTAINING_RECORD(context, IocpOverlapped, context));
        if (iocpOverlapped) {
            if (iocpOverlapped->wsaBuf && iocpOverlapped->bufferCount != count) {
                delete[] iocpOverlapped->wsaBuf;
                iocpOverlapped->wsaBuf = nullptr;
            }
            if (!iocpOverlapped->wsaBuf) {
                iocpOverlapped->wsaBuf = new WSABUF[count];
            }
            for (auto i = 0; i < count; ++i) {
                iocpOverlapped->wsaBuf[i].len = static_cast<ULONG>(vec[i].iov_len);
                iocpOverlapped->wsaBuf[i].buf = reinterpret_cast<char*>(vec[i].iov_base);
            }
            iocpOverlapped->bufferCount = count;
        }
    }
    void bindIoService(socket_type socket) override {
        if (!::CreateIoCompletionPort(reinterpret_cast<HANDLE>(socket), m_iocp, 0, 0)) {
            auto error = GetLastError();
            LOG_ERROR << "CreateIoCompletionPort failed: " << error << " " << formatErrorString(error);
        }
    }
private:
    int m_asyncIoWorkThreadCount;;
    HANDLE m_iocp;
    ThreadPool m_threadPool;

    static LPFN_CONNECTEX s_connectEx;
    static LPFN_ACCEPTEX s_acceptEx;
};

LPFN_CONNECTEX IocpService::s_connectEx = nullptr;
LPFN_ACCEPTEX IocpService::s_acceptEx = nullptr;

} // namespace detail
} // namespace flute

#endif // FLUTE_DETAIL_IOCP_SERVICE_H