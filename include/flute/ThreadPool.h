//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_THREAD_POOL_H
#define FLUTE_THREAD_POOL_H

#include <flute/noncopyable.h>

#include <atomic>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace flute {

class ThreadPool : private noncopyable {
public:
    ThreadPool();
    ~ThreadPool();

    void start(std::size_t size);

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

inline ThreadPool::ThreadPool() : m_running(false), m_workers(), m_tasks(), m_mutex(), m_condition() {}

inline ThreadPool::~ThreadPool() { assert(!m_running); }

inline void ThreadPool::start(std::size_t size) {
    assert(!m_running);
    m_running = true;
    m_workers.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        m_workers.emplace_back(new std::thread(&ThreadPool::run, this));
    }
}

template <typename F, typename... Args>
inline auto ThreadPool::execute(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    auto result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_running) {
            throw std::runtime_error("execute task on not running thread pool.");
        }
        m_tasks.emplace([task]() { (*task)(); });
    }
    m_condition.notify_one();
    return result;
}

inline void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_running = false;
    }
    m_condition.notify_all();
    for (auto t : m_workers) {
        t->join();
        delete t;
    }
    m_workers.clear();
}

inline void ThreadPool::run() {
    for (;;) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_condition.wait(lock, [this] { return !m_running || !m_tasks.empty(); });
            if (!m_running && m_tasks.empty()) {
                return;
            }
            task = std::move(m_tasks.front());
            m_tasks.pop();
        }
        task();
    }
}

} // namespace flute

#endif // FLUTE_THREAD_POOL_H