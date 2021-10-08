#include "../mutex.hpp"

#include <gtest/gtest.h>
#include <pndc/thread_group.hpp>
#include <pndc/timeout.hpp>
#include <barrier>
#include <mutex>
#include <random>

using namespace std::chrono_literals;
using namespace pndc::testing::timeout;

using TestMutex = Mutex;


TEST(Mutex, LockSingleThreaded) {
    TestMutex mutex{};
    pndc::testing::ThreadGroup threads;

    threads.thread([&mutex] {
        for (int i = 0; i < 5; i++) {
            std::lock_guard lock(mutex);
        }
    });

    bool completed = threads.await_completion(1_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded";
}

TEST(Mutex, TryLockSingleThreaded) {
    TestMutex mutex{};
    pndc::testing::ThreadGroup threads;

    threads.thread([&mutex] {
        bool locked = mutex.try_lock();
        ASSERT_TRUE(locked);
        mutex.unlock();
        locked = mutex.try_lock();
        ASSERT_TRUE(locked);
        mutex.unlock();
    });

    bool completed = threads.await_completion(1_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded";
}


class MutexNStepsInCriticalSection : public testing::TestWithParam<uint32_t> {
};

TEST_P(MutexNStepsInCriticalSection, MutualExclusion) {
    TestMutex mutex{};
    std::barrier barrier{2};
    pndc::testing::ThreadGroup threads;
    volatile uint32_t dummy = 0;
    std::mt19937_64 heavy_task{17};

    threads.thread([&mutex, &barrier, &dummy, &heavy_task] {
        std::lock_guard lock(mutex);
        barrier.arrive_and_wait();
        for (uint32_t i = 0; i < GetParam(); i++) {
            dummy = i;
            heavy_task();
        }
    });
    threads.thread([&mutex, &barrier, &dummy] {
        barrier.arrive_and_wait();
        std::lock_guard lock(mutex);
        ASSERT_EQ(dummy, GetParam() - 1) << "Mutual exclusion violated";
    });

    bool completed = threads.await_completion(6_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded";
}

TEST_P(MutexNStepsInCriticalSection, MutualExclusionStress) {
    TestMutex mutex{};
    pndc::testing::ThreadGroup threads;
    volatile uint32_t dummy = 0;
    std::minstd_rand heavy_task{17};

    for (int i = 0; i < 3; i++)
        threads.thread([&mutex, &dummy, &heavy_task] {
            uint32_t iters = GetParam() < 10 ? 128 * 1024 / GetParam() : 8 * 1024 * 1024 / GetParam();
            for (uint32_t j = 0; j < iters; j++) {
                std::lock_guard lock(mutex);
                ASSERT_EQ(dummy, 0) << "Mutual exclusion violated";
                for (uint32_t i = 0; i < GetParam(); i++) {
                    dummy = dummy + 1;
                    heavy_task();
                }
                ASSERT_EQ(dummy, GetParam()) << "Mutual exclusion violated";
                dummy = 0;
            }
        });

    bool completed = threads.await_completion(6_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded";
}

TEST_P(MutexNStepsInCriticalSection, TryLockStress) {
    TestMutex mutex{};
    pndc::testing::ThreadGroup threads;
    volatile uint32_t dummy = 0;
    std::minstd_rand heavy_task{17};

    for (int i = 0; i < 3; i++)
        threads.thread([&mutex, &dummy, &heavy_task] {
            uint32_t iters = GetParam() < 10 ? 128 * 1024 / GetParam() : 8 * 1024 * 1024 / GetParam();
            for (uint32_t j = 0; j < iters; j++) {
                bool lock_waiting = j % 5 == 0;
                bool locked;
                if (lock_waiting) {
                    mutex.lock();
                    locked = true;
                } else {
                    locked = mutex.try_lock();
                }

                if (locked) {
                    std::lock_guard lock(mutex, std::adopt_lock);
                    ASSERT_EQ(dummy, 0) << "Mutual exclusion violated";
                    for (uint32_t i = 0; i < GetParam(); i++) {
                        dummy = dummy + 1;
                        heavy_task();
                    }
                    ASSERT_EQ(dummy, GetParam()) << "Mutual exclusion violated";
                    dummy = 0;
                } else {
                    std::this_thread::yield();
                }
            }
        });

    bool completed = threads.await_completion(6_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded";
}

INSTANTIATE_TEST_SUITE_P(MutexParameterized, MutexNStepsInCriticalSection,
                         testing::Values(1, 100, 1024, 10 * 1024, 8 * 1024 * 1024));


TEST(Mutex, TryLockWhenLocked) {
    TestMutex mutex{};
    std::barrier barrier{2};
    pndc::testing::ThreadGroup threads;

    threads.thread([&mutex, &barrier] {
        {
            std::lock_guard lock(mutex);
            barrier.arrive_and_wait();
            // 1
            barrier.arrive_and_wait();
        }
        barrier.arrive_and_wait();
        // 2
    });

    threads.thread([&mutex, &barrier] {
        barrier.arrive_and_wait();

        // 1
        bool locked = mutex.try_lock();
        ASSERT_FALSE(locked) << "Mutual exclusion violated: try lock unexpectedly succeeded";

        barrier.arrive_and_wait();
        barrier.arrive_and_wait();

        // 2
        locked = mutex.try_lock();
        ASSERT_TRUE(locked) << "Try lock unexpectedly failed";
        mutex.unlock();
    });

    bool completed = threads.await_completion(1_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded";
}
