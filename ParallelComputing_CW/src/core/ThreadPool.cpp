#include "ThreadPool.h"

using namespace std;

ThreadPool::ThreadPool(size_t threads) : stop_(false) {
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                function<void()> task;
                {
                    unique_lock<mutex> lock(this->queue_mutex_);

                    // Чекаємо, поки з'явиться задача або пул зупиниться
                    condition_.wait(lock, [this] {
                        return stop_ || !tasks_.empty();
                    });

                    // Якщо зупинка і черга порожня - виходимо
                    if (stop_ && tasks_.empty()) {
                        return;
                    }
                    // Беремо задачу з черги
                    task = move(tasks_.front());
                    tasks_.pop();
                }
                // Виконуємо задачу
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        unique_lock<mutex> lock(queue_mutex_);
        stop_ = true;
    }

    // Сповіщаємо всі потоки
    condition_.notify_all();

    // Чекаємо завершення всіх потоків
    for (thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

size_t ThreadPool::get_thread_count() const {
    return workers_.size();
}

size_t ThreadPool::get_queue_size() const {
    unique_lock<mutex> lock(queue_mutex_);
    return tasks_.size();
}