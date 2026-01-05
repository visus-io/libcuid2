/// @file cuid2.hpp
/// @brief Main CUID2 identifier generation API
///
/// This is the primary public interface for the libcuid2 library. It provides
/// collision-resistant, sortable unique identifiers suitable for use as database
/// keys, distributed system identifiers, and URL-safe tokens.
///
/// Example usage:
/// @code
///   #include <cuid2/cuid2.hpp>
///
///   // Generate with default length (24 characters)
///   std::string id = visus::cuid2::generate();
///
///   // Generate with custom length (16 characters)
///   std::string short_id = visus::cuid2::generate(16);
/// @endcode

#ifndef LIBCUID2_CUID2_HPP
#define LIBCUID2_CUID2_HPP

#include <cuid2/cuid2_export.hpp>
#include <stdexcept>
#include <string>

namespace visus::cuid2 {
    /// Default CUID2 identifier length in characters.
    /// Provides a good balance between compactness and collision resistance.
    constexpr int DEFAULT_LENGTH = 24;

    /// Minimum allowed CUID2 identifier length.
    /// Shorter identifiers have significantly higher collision probability.
    constexpr int MIN_CUID2_LENGTH = 4;

    /// Maximum allowed CUID2 identifier length.
    /// Longer identifiers provide more entropy but are less compact.
    constexpr int MAX_CUID2_LENGTH = 32;

    /// Generates a CUID2 identifier of the specified length.
    ///
    /// Creates a collision-resistant, sortable unique identifier by combining:
    /// - Current timestamp (for sortability)
    /// - Atomic counter (for uniqueness within same timestamp)
    /// - System fingerprint (for uniqueness across processes/machines)
    /// - Cryptographic random bytes (for collision resistance)
    /// - Random prefix character (ensures valid identifier in most languages)
    ///
    /// All components are hashed with NIST FIPS-202 SHA3-512 and encoded as
    /// base-36 for a compact, URL-safe representation.
    ///
    /// @param MAX_LENGTH Desired identifier length (default: 24, min: 4, max: 32)
    /// @return A CUID2 identifier string of exact length MAX_LENGTH
    /// @throws std::invalid_argument if MAX_LENGTH is outside valid range [4, 32]
    /// @note Thread-safe: Can be called concurrently from multiple threads
    CUID2_API std::string generate(int MAX_LENGTH = DEFAULT_LENGTH);
} // namespace visus::cuid2

#endif //LIBCUID2_CUID2_HPP
