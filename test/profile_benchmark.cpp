#include "Public/profile/profile.h"

#include <chrono>
#include <iostream>

namespace {

volatile U64 consumedBytes = 0;

template <typename Function>
double nsPerIteration(U32 iterations, Function &&function) {
    const auto start = std::chrono::steady_clock::now();
    for (U32 index = 0; index < iterations; ++index) {
        function();
    }
    const auto end = std::chrono::steady_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    return static_cast<double>(elapsed) / iterations;
}

} // namespace

int main() {
    constexpr U32 warmupIterations = 10000;
    constexpr U32 benchmarkIterations = 1000000;
    ProfileLabel staticLabel("benchmark.scope");

    for (U32 index = 0; index < warmupIterations; ++index) {
        ProfileScope scope(staticLabel);
    }

    const double clockPairNs = nsPerIteration(benchmarkIterations, [] {
        const auto start = ProfileScope::Clock::now();
        const auto end = ProfileScope::Clock::now();
        consumedBytes += static_cast<U64>(end >= start);
    });
    const double dynamicEnabledNs = nsPerIteration(benchmarkIterations, [] {
        ProfileScope scope("benchmark.scope");
    });
    const double staticEnabledNs = nsPerIteration(benchmarkIterations, [&] {
        ProfileScope scope(staticLabel);
    });
    const double staticMacroNs = nsPerIteration(benchmarkIterations, [] {
        AOE_PROFILE_SCOPE("benchmark.macro_scope");
    });

    std::cout << "clock_pair_ns=" << clockPairNs << '\n'
              << "profile_dynamic_ns=" << dynamicEnabledNs << '\n'
              << "profile_static_label_ns=" << staticEnabledNs << '\n'
              << "profile_static_macro_ns=" << staticMacroNs << '\n'
              << "static_overhead_excluding_clocks_ns=" << (staticEnabledNs - clockPairNs) << '\n'
              << "consumed_bytes=" << consumedBytes << '\n';
    return 0;
}
