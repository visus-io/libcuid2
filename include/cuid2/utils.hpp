/// @file utils.hpp
/// @brief Utility functions for CUID2 generation
///
/// Provides helper functions for base-36 encoding, timestamp generation,
/// and random prefix character generation used by the CUID2 algorithm.

#ifndef LIBCUID2_UTILS_HPP
#define LIBCUID2_UTILS_HPP

#include <concepts>
#include <cstdint>
#include <span>
#include <string>

namespace visus::cuid2::utils {
    /// Concept for byte-like types (uint8_t, unsigned char, std::byte).
    /// Used to constrain functions that operate on raw byte data.
    template<typename T>
    concept ByteLike = std::same_as<std::remove_cv_t<T>, uint8_t> ||
                       std::same_as<std::remove_cv_t<T>, unsigned char> ||
                       std::same_as<std::remove_cv_t<T>, std::byte>;

    /// Concept for contiguous byte ranges suitable for cryptographic operations.
    /// Ensures the type is a contiguous range of byte-like elements.
    template<typename T>
    concept ByteRange = std::ranges::contiguous_range<T> &&
                        ByteLike<std::ranges::range_value_t<T>>;

    /// Generates a random lowercase letter prefix for CUID2 identifiers.
    ///
    /// Uses cryptographically secure randomness to select a letter from a-z.
    /// This ensures CUID2 identifiers always start with a letter, making them
    /// valid identifiers in most programming languages.
    ///
    /// @return A random lowercase letter from 'a' to 'z'
    /// @note Thread-safe: Can be called concurrently from multiple threads
    [[nodiscard]] char generate_prefix() noexcept;

    /// Encodes a byte array as a base-36 string.
    ///
    /// Converts an arbitrary-length byte array into a base-36 encoded string
    /// using Boost.Multiprecision. The input bytes are interpreted as a
    /// big-endian unsigned integer.
    ///
    /// @param data Span of bytes to encode (interpreted as big-endian)
    /// @return Base-36 encoded string, or "0" if input is empty or all zeros
    [[nodiscard]] std::string encode_base36(std::span<const uint8_t> data);

    /// Returns the current time as 100-nanosecond ticks since Unix epoch.
    ///
    /// Provides a high-resolution timestamp suitable for sortable identifier
    /// generation. The value represents the number of 100-nanosecond intervals
    /// since January 1, 1970 00:00:00 UTC (Unix epoch).
    ///
    /// @return Current timestamp as 100-nanosecond ticks since Unix epoch
    /// @note Thread-safe: Can be called concurrently from multiple threads
    [[nodiscard]] int64_t get_timestamp_ticks() noexcept;
} // namespace visus::cuid2::utils

#endif // LIBCUID2_UTILS_HPP
