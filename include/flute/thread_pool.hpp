/*************************************************************************
 *
 * File Name:  thread_pool.hpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26 01:06:13
 *
 *************************************************************************/

#pragma once
#include <flute/noncopyable.hpp>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace flute {

class thread_pool : private noncopyable {
public:
    thread_pool();
    ~thread_pool();

    void start(int size);

    template <typename F, typename... Args>
    auto execute(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    void shutdown();

private:
    std::atomic<bool> m_running;
    std::vector<std::thread*> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    std::condition_variable m_condition;

    void run();
};

#include <flute/thread_pool.inl>

} // namespace flute