#include "thread_pool.hpp"

namespace sage {

ThreadPool::ThreadPool(std::size_t worker_count) {
    if (worker_count == 0) {
        worker_count = 1;
    }
    workers_.reserve(worker_count);
    for (std::size_t i = 0; i < worker_count; ++i) {
        workers_.emplace_back(&ThreadPool::worker_loop, this);
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

std::size_t ThreadPool::size() const {
    return workers_.size();
}

void ThreadPool::wait() {
    std::unique_lock<std::mutex> lock(pending_mutex_);
    pending_cv_.wait(lock, [this]() {
        return tasks_pending_ == 0;
    });
}

void ThreadPool::worker_loop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            cv_.wait(lock, [this]() {
                return stop_ || !tasks_.empty();
            });
            if (stop_ && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }
}

} // namespace sage
