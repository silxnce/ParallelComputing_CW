#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>

class ThreadPool {
public:
    explicit ThreadPool(size_t threads);
    ~ThreadPool();

    template <typename F>
    auto enqueue(F&& task) -> std::future<void>;
    size_t get_thread_count() const;
    size_t get_queue_size() const;

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
};

template <typename F>
auto ThreadPool::enqueue(F&& task)
    -> std::future<void>
{
    auto packaged = std::make_shared<std::packaged_task<void()>>(
        std::forward<F>(task)
    );

    std::future<void> result = packaged->get_future();

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        if (stop_)
            throw std::runtime_error("ThreadPool stopped");

        tasks_.emplace([packaged]() { (*packaged)(); });
    }

    condition_.notify_one();
    return result;
}

#endif //THREADPOOL_H
