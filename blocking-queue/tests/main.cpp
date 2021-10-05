#include "../blocking_queue.hpp"

#include <pndc/samples.hpp>
#include <pndc/thread_group.hpp>
#include <gtest/gtest.h>

#include <deque>
#include <optional>
#include <chrono>

using namespace std::chrono_literals;


TEST(BlockingQueueTests, SingleProducerSingleConsumer) {
    pndc::testing::ThreadGroup threads;
    pndc::testing::SampleGenerator sample_gen;

    const auto samples = sample_gen.random_ints(1'000'000);
    BlockingQueue<int> queue(10);

    threads.thread([&samples, &queue] {
        for (int v : samples) {
            queue.put(v);
        }
    });
    threads.thread([&samples, &queue] {
        size_t i = 0;
        std::optional<int> got;
        while ((got = queue.take()).has_value()) {
            ASSERT_EQ(samples[i++], got);
        }
    });

    bool completed = threads.await_completion(2s);
    ASSERT_TRUE(completed) << "Timeout exceeded" << std::endl;
}
