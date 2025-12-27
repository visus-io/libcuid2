/// @file fingerprint.hpp
/// @brief System fingerprint for CUID2 generation
///
/// Provides a singleton fingerprint that uniquely identifies the current system
/// and process. The fingerprint combines hostname, process ID, and environment
/// variables into a deterministic byte sequence.

#ifndef LIBCUID2_FINGERPRINT_HPP
#define LIBCUID2_FINGERPRINT_HPP

#include <cstdint>
#include <vector>

namespace visus::cuid2 {
    /// System fingerprint singleton for CUID2 generation.
    ///
    /// This class uses C++17 inline static variable for singleton implementation
    /// with a cached fingerprint value computed once during initialization. The
    /// fingerprint combines hostname, process ID (little-endian), and sorted
    /// environment variables to create a unique identifier for the system/process.
    class Fingerprint {
        /// Singleton instance with thread-safe initialization.
        ///
        /// @note Defined outside class after type is complete
        static Fingerprint instance_;

        /// Cached fingerprint byte sequence, computed once during construction.
        std::vector<uint8_t> cached_value_;

        /// Private constructor, computes and caches the fingerprint.
        Fingerprint();

    public:
        /// Returns the cached system fingerprint.
        ///
        /// @return Const reference to the fingerprint byte vector
        /// @note Thread-safe: Can be called concurrently from multiple threads
        [[nodiscard]] static const std::vector<uint8_t>& get();
    };

    /// Inline static definition of singleton instance.
    /// Must be defined after the class is complete.
    inline Fingerprint Fingerprint::instance_{};
} // namespace visus::cuid2

#endif // LIBCUID2_FINGERPRINT_HPP