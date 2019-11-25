/*************************************************************************
 *
 * File Name:  thread_pool.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/26 01:06:22
 *
 *************************************************************************/

#include <flute/thread_pool.hpp>

#include <cassert>

namespace flute {

thread_pool::thread_pool() : m_running(false), m_workers(), m_tasks(), m_mutex(), m_condition() {
}

thread_pool::~thread_pool() {
    assert(!m_running);
}

void thread_pool::start(int size) {
    assert(!m_running);
    m_running = true;
    m_workers.reserve(size);
    for (auto i = 0; i < size; ++i) {
        m_workers.emplace_back(new std::thread(&thread_pool::run, this));
    }
}

template <typename F, typename... Args>
auto thread_pool::execute(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = std::result_of<F(Args...)>::type;
    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)));
    s auto result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_running) {
            throw std::runtime_error("execute task on not running thread pool.");
        }
        m_tasks.push([task]() { (*task)(); });
    }
    m_condition.notify_one();
}

void thread_pool::shutdown() {
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

void thread_pool::run() {
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