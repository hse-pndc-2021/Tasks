#pragma once

#include <random>
#include <vector>

namespace pndc::testing {
class SampleGenerator {
public:
    std::vector<int> random_ints(size_t n) {
        return generate_ints(n, [this] { return engine_(); });
    }

    std::vector<int> serial_ints(size_t n) {
        int counter = 0;
        return generate_ints(n, [&counter]() mutable { return ++counter; });
    }

private:
    template<class F>
    std::vector<int> generate_ints(size_t n, F generator) {
        std::vector<int> res(n, 0);
        std::generate(res.begin(), res.end(), std::move(generator));
        return res;
    }

private:
    std::minstd_rand engine_{17};
};
}