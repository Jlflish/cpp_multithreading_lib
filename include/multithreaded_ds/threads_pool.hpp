#pragma once

#include <future>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace multithreaded_ds {
class thread_pool {
public:
    thread_pool(size_t threads = 8) : thread_count(threads), stop(false) {
        workers.reserve(threads);
        for (size_t i = 0; i < threads; i++) {
            workers.push_back(std::thread(thread_pool::worker_thread, this));
        }
    }

    ~thread_pool() {
        {
            std::unique_lock<std::mutex> lock(queue_mtx);
            stop.store(true, std::memory_order_release);
        }
        cv.notify_all();
        for (auto &i : workers) {
            if (i.joinable()) {
                i.join();
            }
        }
    }

    template <typename F, typename... Args>
    auto submit(F &&f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;

        std::shared_ptr<std::promise<return_type>> promise = std::make_shared<std::promise<return_type>>();
        std::future<return_type> future = promise->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mtx);

            task_queue.push(
                [ promise, func = std::forward<F>(f),
                  tup = std::make_tuple(std::forward<Args>(args)...) ]() mutable
                {
                    try {
                        if constexpr(std::is_void_v<return_type>) {
                            std::apply(func, tup);
                            promise->set_value();
                        } else {
                            auto result = std::apply(func, tup);
                            promise->set_value(std::move(result));
                        }
                    } catch(...) {
                        promise->set_exception(std::current_exception());
                    }
                }
            );
        }

        cv.notify_one();
        return future;
    }

private:
    void worker_thread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mtx);

                cv.wait(lock, [this] { return !task_queue.empty() || stop; });

                if (stop.load(std::memory_order_acquire) && task_queue.empty()) {
                    return;
                }

                task = std::move(task_queue.front());
                task_queue.pop();
            }
            task();
            cv.notify_all();
        }
    }
    size_t thread_count;
    std::mutex queue_mtx;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> task_queue;
    std::condition_variable cv;
    std::atomic<bool> stop;
};
} // namespace multithreaded_ds 