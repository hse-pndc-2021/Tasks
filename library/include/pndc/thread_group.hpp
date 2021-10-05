#pragma once

#include <concepts>
#include <functional>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace pndc::testing {

class DynamicLatch {
public:
    DynamicLatch() = default;

    DynamicLatch(const DynamicLatch&) = delete;

    DynamicLatch(DynamicLatch&&) = delete;

    DynamicLatch& operator=(const DynamicLatch&) = delete;

    DynamicLatch& operator=(DynamicLatch&&) = delete;

    void await(uint32_t n) {
        std::unique_lock lock(completion_mutex_);
        while (completed_ < n) {
            completion_condition_.wait(lock);
        }
    }

    template<class Rep, class Period>
    bool await(uint32_t n, const std::chrono::duration<Rep, Period>& timeout) {
        auto deadline = std::chrono::steady_clock::now() + timeout;

        std::unique_lock lock(completion_mutex_);
        while (completed_ < n) {
            auto status = completion_condition_.wait_until(lock, deadline);
            if (status == std::cv_status::timeout) {
                return false;
            }
        }
        return true;
    }

    void satisfy() {
        std::lock_guard lock(completion_mutex_);
        completed_++;
        completion_condition_.notify_all();
    }

private:
    std::mutex completion_mutex_{};
    std::condition_variable completion_condition_{};
    uint32_t completed_ = 0;
};

class ThreadGroup {
public:
    ThreadGroup() = default;

    ThreadGroup(const ThreadGroup&) = delete;

    ThreadGroup(ThreadGroup&&) = default;

    ThreadGroup& operator=(const ThreadGroup&) = delete;

    ThreadGroup& operator=(ThreadGroup&&) = default;

    template<std::invocable T>
    void thread(T func) {
        threads_.emplace_back([func = std::move(func), completion = completion_latch_] {
            std::invoke(std::move(func));
            completion->satisfy();
        });
    }

    template<class Rep, class Period>
    bool await_completion(const std::chrono::duration<Rep, Period>& timeout) {
        return completion_latch_->await(threads_.size(), timeout);
    }

    ~ThreadGroup() noexcept {
        for (std::thread& thread: threads_) {
            thread.join();
        }
    }

private:
    std::vector<std::thread> threads_{};
    std::shared_ptr<DynamicLatch> completion_latch_ = std::make_shared<DynamicLatch>();
};
}
