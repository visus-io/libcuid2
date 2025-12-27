/// @file counter.cpp
/// @brief Thread-safe atomic counter implementation for CUID2 generation
///
/// This file implements a singleton counter that provides monotonically increasing
/// values for CUID2 identifier generation. The counter is initialized with a
/// cryptographically random seed to ensure uniqueness across process restarts.

#include "cuid2/counter.hpp"

#include <array>
#include <cstdint>
#include <ranges>

#include "cuid2/platform.hpp"

namespace visus::cuid2 {
    /// Multiplier applied to random seed for counter initialization.
    /// This prime number helps distribute the initial counter values across
    /// a large range to minimize collision probability.
    constexpr int64_t COUNTER_SEED_MULTIPLIER = 476782367;

    namespace {
        /// Number of bits in a byte, used for bit-shifting operations.
        constexpr size_t BITS_PER_BYTE = 8;

        /// Generates the initial counter value using cryptographic randomness.
        ///
        /// Creates a random 64-bit seed using the platform's CSPRNG and multiplies
        /// it by COUNTER_SEED_MULTIPLIER to create the initial counter value.
        /// This ensures different processes start with different counter values,
        /// reducing collision probability.
        ///
        /// @return A randomly initialized int64_t value to seed the counter
        /// @throws May throw if the platform CSPRNG fails (rare)
        int64_t generate_initial_counter_value() {
            std::array<uint8_t, sizeof(int64_t)> bytes{};
            platform::get_random_bytes(bytes.data(), bytes.size());

            int64_t seed = 0;
            size_t shift = 0;
            for (const auto BYTE : bytes | std::views::all) {
                seed |= static_cast<int64_t>(BYTE) << shift;
                shift += BITS_PER_BYTE;
            }

            return seed * COUNTER_SEED_MULTIPLIER;
        }
    }

    /// Constructs the Counter singleton with a cryptographically random initial value.
    ///
    /// The counter is initialized using generate_initial_counter_value(), which
    /// provides a random seed multiplied by COUNTER_SEED_MULTIPLIER. This ensures
    /// different processes have different starting counter values.
    ///
    /// @note This constructor is private and only called once by the singleton pattern
    Counter::Counter() : value_{generate_initial_counter_value()} {
    }

    /// Returns the singleton Counter instance.
    ///
    /// This function implements the Meyers singleton pattern with thread-safe
    /// initialization guaranteed by C++11. The static local variable is initialized
    /// exactly once, even when called concurrently from multiple threads.
    ///
    /// @return Reference to the singleton Counter instance
    /// @note Thread-safe initialization guaranteed by C++11 static local variables
    Counter& Counter::instance() {
        static Counter instance;
        return instance;
    }

    /// Returns the next counter value in a thread-safe manner.
    ///
    /// This static method accesses the singleton Counter instance and atomically
    /// increments its value, returning the pre-increment value. The operation is
    /// lock-free on most platforms and safe for concurrent access.
    ///
    /// @return The next sequential counter value (monotonically increasing)
    /// @note Thread-safe: Can be called concurrently from multiple threads
    int64_t Counter::next() {
        return instance().value_.fetch_add(1);
    }
} // namespace visus::cuid2
