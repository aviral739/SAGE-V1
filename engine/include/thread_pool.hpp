#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <type_traits>
#include <stdexcept>
#include <cstddef>

namespace sage {

class ThreadPool {
public:
    explicit ThreadPool(std::size_t worker_count);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    std::size_t size() const;
    void wait();

    template <typename Func, typename... Args>
    auto submit(Func&& f, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using return_type = std::invoke_result_t<Func, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
        );

        auto wrapper = [this, task]() {
            (*task)();
            {
                std::lock_guard<std::mutex> lock(pending_mutex_);
                --tasks_pending_;
            }
            pending_cv_.notify_one();
        };

        {
            std::lock_guard<std::mutex> queue_lock(queue_mutex_);

            if (stop_) {
                throw std::runtime_error("Cannot submit task to stopped ThreadPool");
            }

            {
                std::lock_guard<std::mutex> pending_lock(pending_mutex_);
                ++tasks_pending_;
            }

            tasks_.push(std::move(wrapper));
        }

        cv_.notify_one();
        return task->get_future();
    }

private:
    void worker_loop();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::mutex pending_mutex_;
    std::condition_variable pending_cv_;
    std::size_t tasks_pending_ = 0;
    bool stop_ = false;
};

} // namespace sage
