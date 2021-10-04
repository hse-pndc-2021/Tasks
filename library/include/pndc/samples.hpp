#pragma once

#include <random>
#include <vector>

namespace pndc::testing {
class SampleGenerator {
public:
    std::vector<int> next_samples(size_t n) {
        std::vector<int> res(n, 0);
        std::generate(res.begin(), res.end(), [this] { return engine_(); });
        return res;
    }

private:
    std::minstd_rand engine_;
    std::uniform_int_distribution<int> distribution_;
};
}