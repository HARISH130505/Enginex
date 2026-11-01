#include "workbench/performance/thread_pool.hpp"

namespace enginex::performance {

ThreadPool::ThreadPool(size_t num_threads) {
    if (num_threads == 0) num_threads = 1;
    m_workers.reserve(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        m_workers.emplace_back([this]() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->m_queue_mutex);
                    this->m_cv.wait(lock, [this]() {
                        return this->m_stop.load() || !this->m_tasks.empty();
                    });

                    if (this->m_stop.load() && this->m_tasks.empty()) {
                        return;
                    }

                    task = std::move(this->m_tasks.front());
                    this->m_tasks.pop();
                    this->m_active_tasks.fetch_add(1, std::memory_order_relaxed);
                }

                // Execute task outside the lock
                try {
                    task();
                } catch (...) {
                    // Suppress exceptions from worker loop
                }

                {
                    std::unique_lock<std::mutex> lock(this->m_queue_mutex);
                    this->m_active_tasks.fetch_sub(1, std::memory_order_relaxed);
                    if (this->m_tasks.empty() && this->m_active_tasks.load() == 0) {
                        this->m_wait_cv.notify_all();
                    }
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stop.store(true);
    }
    m_cv.notify_all();

    for (std::thread& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::wait_all() {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    m_wait_cv.wait(lock, [this]() {
        return this->m_tasks.empty() && this->m_active_tasks.load() == 0;
    });
}

} // namespace enginex::performance

