#pragma once

#include <chrono>

#ifndef TIMEOUT_SCALE_MULTIPLIER
#define TIMEOUT_SCALE_MULTIPLIER 1
#endif

namespace pndc::testing {
namespace {

template<class Rep, class Period>
constexpr auto scale_timeout(const std::chrono::duration<Rep, Period>& duration) {
    return duration * TIMEOUT_SCALE_MULTIPLIER;
}

}

namespace timeout {

constexpr auto operator ""_timeout_s(unsigned long long secs) {
    return scale_timeout(std::chrono::seconds(secs));
}

constexpr auto operator ""_timeout_ms(unsigned long long msecs) {
    return scale_timeout(std::chrono::milliseconds(msecs));
}

}
}