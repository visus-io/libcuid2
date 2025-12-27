/// @file counter.hpp
/// @brief Thread-safe atomic counter for CUID2 generation
///
/// Provides a singleton counter that generates monotonically increasing values
/// for CUID2 identifier generation. The counter is initialized with a
/// cryptographically random seed to ensure uniqueness across process restarts.

#ifndef LIBCUID2_COUNTER_HPP
#define LIBCUID2_COUNTER_HPP

#include <atomic>
#include <cstdint>

namespace visus::cuid2 {
    /// Thread-safe atomic counter singleton for CUID2 generation.
    ///
    /// This class uses C++17 inline static variable for singleton implementation
    /// with a cryptographically random initial value. The counter provides
    /// monotonically increasing values that ensure uniqueness within the same
    /// timestamp across concurrent requests.
    class Counter {
        /// Singleton instance with thread-safe initialization.
        ///
        /// @note Defined outside class after type is complete
        static Counter instance_;

        /// Atomic counter value, safe for concurrent access.
        std::atomic<int64_t> value_;

        /// Private constructor, initializes counter with random seed.
        Counter();

    public:
        /// Returns the next counter value in a thread-safe manner.
        ///
        /// @return The next sequential counter value
        /// @note Thread-safe: Can be called concurrently from multiple threads
        [[nodiscard]] static int64_t next();
    };

    /// Inline static definition of singleton instance.
    /// Must be defined after the class is complete.
    inline Counter Counter::instance_{};
} // namespace visus::cuid2

#endif // LIBCUID2_COUNTER_HPP
