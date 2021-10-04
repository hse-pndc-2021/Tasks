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

    const auto samples = sample_gen.next_samples(1'000'000);
    BlockingQueue queue(10);

    threads.thread([&samples] {
        for (int v : samples) {
            queue.put(v);
        }
    });
    threads.thread([&samples] {
        size_t i = 0;
        std::optional<int> got;
        while ((got = queue.take()).has_value()) {
            ASSERT_EQ(samples[i++], got);
        }
    });

    threads.await_completion(1s);
}
