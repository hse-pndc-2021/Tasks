#include "../blocking_queue.hpp"

#include <pndc/timeout.hpp>
#include <pndc/samples.hpp>
#include <pndc/thread_group.hpp>
#include <gtest/gtest.h>

#include <algorithm>
#include <unordered_set>
#include <optional>
#include <chrono>

using namespace std::chrono_literals;
using namespace pndc::testing::timeout;


TEST(BlockingQueueTests, SingleProducerSingleConsumer) {
    BlockingQueue<int> queue(10);
    pndc::testing::SampleGenerator sample_gen;
    const auto samples = sample_gen.random_ints(1'000'000);
    pndc::testing::ThreadGroup threads;

    threads.thread([&samples, &queue] {
        for (int v: samples) {
            queue.put(v);
        }
        queue.close();
    });
    threads.thread([&samples, &queue] {
        size_t i = 0;
        std::optional<int> got;
        while ((got = queue.take()).has_value()) {
            ASSERT_EQ(samples[i++], *got) << "Wrong element";
        }
        ASSERT_EQ(i, samples.size()) << "Queue looses / duplicates elements";
    });

    bool completed = threads.await_completion(2_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;
}

TEST(BlockingQueueTests, SingleProducerMultiConsumer) {
    constexpr size_t NUM_CONSUMERS = 3;
    constexpr int NUM_SAMPLES = 1'000'000;

    BlockingQueue<int> queue(10);
    pndc::testing::SampleGenerator sample_gen;
    pndc::testing::ThreadGroup threads;

    threads.thread([&queue] {
        for (int i = 0; i < NUM_SAMPLES; i++) {
            queue.put(i);
        }

        queue.close();
    });

    std::vector<std::future<std::vector<int>>> results;
    for (size_t i = 0; i < NUM_CONSUMERS; i++) {
        auto res_future = threads.async([&queue] {
            std::vector<int> result;
            std::optional<int> got;
            while ((got = queue.take()).has_value()) {
                result.push_back(*got);
            }
            return result;
        });
        results.emplace_back(std::move(res_future));
    }

    bool completed = threads.await_completion(3_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;

    std::unordered_set<int> total_res;
    for (auto& res_future: results) {
        std::vector<int> res = res_future.get();
        ASSERT_TRUE(std::is_sorted(res.begin(), res.end())) << "Queue violates FIFO order";
        total_res.insert(res.begin(), res.end());
    }

    ASSERT_EQ(NUM_SAMPLES, total_res.size()) << "Queue looses / duplicates elements";
}

TEST(BlockingQueueTests, MultiProducerMultiConsumer) {
    constexpr size_t NUM_PRODUCERS = 3;
    constexpr size_t NUM_CONSUMERS = 3;
    constexpr int NUM_SAMPLES = 3'000'000;

    BlockingQueue<int> queue(30);
    pndc::testing::SampleGenerator sample_gen;
    const auto samples = sample_gen.serial_ints(NUM_SAMPLES);
    std::atomic<size_t> index{0};
    std::atomic<size_t> count{0};
    pndc::testing::ThreadGroup threads;

    for (size_t i = 0; i < NUM_PRODUCERS; i++) {
        threads.thread([&samples, &index, &queue, &count] {
            size_t i;
            while ((i = index.fetch_add(1, std::memory_order_relaxed)) < samples.size()) {
                if (queue.put(samples[i])) {
                    count.fetch_add(1, std::memory_order_relaxed);
                } else {
                    break;
                }
            }

            ASSERT_TRUE(i >= samples.size() || queue.is_closed());
        });
    }

    std::vector<std::future<std::vector<int>>> results;
    for (size_t i = 0; i < NUM_CONSUMERS; i++) {
        auto res_future = threads.async([&queue] {
            std::vector<int> result;
            std::optional<int> got;
            while ((got = queue.take()).has_value()) {
                result.push_back(*got);
            }
            return result;
        });
        results.emplace_back(std::move(res_future));
    }

    threads.await_completion(1s);
    queue.close();

    bool completed = threads.await_completion(300_timeout_ms);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;

    std::unordered_set<int> total_res;
    for (auto& res_future: results) {
        std::vector<int> res = res_future.get();
        total_res.insert(res.begin(), res.end());
    }

    ASSERT_EQ(count.load(), total_res.size()) << "Queue looses / duplicates elements";
}

TEST(BlockingQueueTests, ConsumerBlocking) {
    BlockingQueue<int> queue(10);
    pndc::testing::ThreadGroup threads;

    threads.async([&queue] {
        queue.put(1);
        queue.put(2);
    });
    bool completed = threads.await_completion(300_timeout_ms);
    ASSERT_TRUE(completed) << "Timeout exceeded";

    threads.thread([&queue] {
        ASSERT_EQ(1, queue.take());
        ASSERT_EQ(2, queue.take());
        ASSERT_EQ(3, queue.take());
    });
    completed = threads.await_completion(200ms);
    ASSERT_FALSE(completed) << "Consumer was expected to block";

    threads.thread([&queue] {
        queue.put(3);
    });

    completed = threads.await_completion(300_timeout_ms);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;
}

TEST(BlockingQueueTests, ProducerBlocking) {
    BlockingQueue<int> queue(3);
    pndc::testing::ThreadGroup threads;

    threads.thread([&queue] {
        queue.put(1);
        queue.put(2);
        queue.put(3);
    });
    bool completed = threads.await_completion(300_timeout_ms);
    ASSERT_TRUE(completed) << "Timeout exceeded";

    threads.thread([&queue] {
        queue.put(4);
    });
    completed = threads.await_completion(200ms);
    ASSERT_FALSE(completed) << "Producer was expected to block";

    threads.thread([&queue] {
        ASSERT_EQ(1, queue.take());
    });

    completed = threads.await_completion(300_timeout_ms);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;

    threads.thread([&queue] {
        ASSERT_EQ(2, queue.take());
        ASSERT_EQ(3, queue.take());
        ASSERT_EQ(4, queue.take());
    });

    completed = threads.await_completion(300_timeout_ms);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;
}

TEST(BlockingQueueTests, SingleThread) {
    pndc::testing::ThreadGroup threads;

    threads.thread([] {
        BlockingQueue<int> queue(4);
        queue.put(1);
        queue.put(2);
        ASSERT_TRUE(queue.offer(3));
        ASSERT_EQ(1, queue.take());
        ASSERT_EQ(2, queue.take());
        queue.put(1);
        queue.put(2);
        queue.put(3);
        ASSERT_FALSE(queue.offer(3));
        ASSERT_EQ(3, queue.take());
        ASSERT_EQ(1, queue.take());
        ASSERT_EQ(2, queue.take());
        ASSERT_EQ(3, queue.take());
    });

    bool completed = threads.await_completion(1_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;
}

TEST(BlockingQueueTests, QueueClose) {
    pndc::testing::ThreadGroup threads;

    threads.thread([] {
        BlockingQueue<int> queue(4);
        ASSERT_FALSE(queue.is_closed());
        queue.close();
        ASSERT_TRUE(queue.is_closed());
        ASSERT_FALSE(queue.put(5));
        ASSERT_EQ(std::nullopt, queue.take());
        ASSERT_FALSE(queue.offer(5));
        ASSERT_EQ(std::nullopt, queue.take());
    });

    bool completed = threads.await_completion(1_timeout_s);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;
}

TEST(BlockingQueueTests, ConcurrentQueueClose) {
    BlockingQueue<int> queue(256);
    pndc::testing::ThreadGroup threads;

    threads.thread([&queue] {
        int i = 0;
        while (queue.put(i++));
        ASSERT_TRUE(queue.is_closed());
    });
    threads.thread([&queue] {
        while (!queue.is_closed());
    });

    bool completed = threads.await_completion(100ms);
    ASSERT_FALSE(completed) << "Queue unexpectedly stopped accepting elements";

    queue.close();

    completed = threads.await_completion(300_timeout_ms);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;
}
