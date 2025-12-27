/// @file utils.cpp
/// @brief Utility functions for CUID2 generation
///
/// This file provides helper functions for CUID2 identifier generation including:
/// - Base-36 encoding using Boost.Multiprecision for arbitrary precision
/// - Timestamp generation in 100-nanosecond ticks
/// - Random prefix character generation (a-z)

#include "cuid2/utils.hpp"

#include <algorithm>
#include <chrono>
#include <ranges>
#include <stdexcept>
#include <string_view>

#include <boost/multiprecision/cpp_int.hpp>

#include "cuid2/platform.hpp"

namespace visus::cuid2::utils {
    using boost::multiprecision::cpp_int;

    namespace {
        /// Number of lowercase letters in the English alphabet (a-z).
        constexpr uint8_t LOWERCASE_LETTER_COUNT = 26;

        /// Base-36 radix for encoding (0-9, a-z).
        constexpr int BASE36_RADIX = 36;

        /// Number of bits in a byte, used for bit-shifting operations.
        constexpr uint8_t BITS_PER_BYTE = 8;

        /// Number of 100-nanosecond ticks per second.
        /// Used for high-resolution timestamp generation with cross-platform compatibility.
        constexpr int64_t TICKS_PER_SECOND = 10'000'000;

        /// Returns the base-36 character set as a compile-time constant.
        ///
        /// The character set consists of digits 0-9 followed by lowercase letters a-z,
        /// providing 36 unique characters for base-36 encoding.
        ///
        /// @return A string_view containing "0123456789abcdefghijklmnopqrstuvwxyz"
        consteval std::string_view get_base36_chars() noexcept {
            return "0123456789abcdefghijklmnopqrstuvwxyz";
        }

        /// Converts a random byte to a lowercase letter (a-z).
        ///
        /// Maps a random byte value to one of 26 lowercase letters using modulo
        /// arithmetic. This ensures uniform distribution across the alphabet.
        ///
        /// @param random_byte A random byte value
        /// @return A lowercase letter from 'a' to 'z'
        constexpr char prefix_from_byte(uint8_t random_byte) noexcept {
            return static_cast<char>('a' + (random_byte % LOWERCASE_LETTER_COUNT));
        }
    }

    /// Generates a random lowercase letter prefix for CUID2 identifiers.
    ///
    /// Uses a cryptographically secure random byte from the platform CSPRNG
    /// to select a random letter from a-z. This ensures CUID2 identifiers
    /// always start with a letter, making them valid identifiers in most
    /// programming languages.
    ///
    /// @return A random lowercase letter from 'a' to 'z'
    /// @note Thread-safe: Can be called concurrently from multiple threads
    char generate_prefix() noexcept {
        uint8_t random_byte = 0;
        platform::get_random_bytes(&random_byte, 1);

        return prefix_from_byte(random_byte);
    }

    /// Encodes a byte array as a base-36 string.
    ///
    /// Converts an arbitrary-length byte array into a base-36 encoded string
    /// using Boost.Multiprecision for arbitrary precision arithmetic. The input
    /// bytes are interpreted as a big-endian unsigned integer, then converted
    /// to base-36 using digits 0-9 and letters a-z.
    ///
    /// @param data Span of bytes to encode (interpreted as big-endian)
    /// @return Base-36 encoded string, or "0" if input is empty or all zeros
    std::string encode_base36(std::span<const uint8_t> data) {
        if (data.empty()) [[unlikely]] {
            return "0";
        }

        cpp_int num = 0;
        for (uint8_t const BYTE : data) {
            num = (num << BITS_PER_BYTE) | BYTE;
        }

        if (num == 0) [[unlikely]] {
            return "0";
        }

        constexpr auto BASE36_CHARS = get_base36_chars();
        std::string result;

        result.reserve(data.size() * 2);

        while (num > 0) {
            cpp_int const REMAINDER = num % BASE36_RADIX;
            result.push_back(BASE36_CHARS[static_cast<int>(REMAINDER)]);
            num /= BASE36_RADIX;
        }

        std::ranges::reverse(result);

        return result;
    }

    /// Returns the current time as 100-nanosecond ticks since Unix epoch.
    ///
    /// Provides a high-resolution timestamp as the number of 100-nanosecond
    /// intervals since January 1, 1970 00:00:00 UTC (Unix epoch). The timestamp
    /// is monotonically increasing (within clock precision) and suitable for
    /// sortable identifier generation.
    ///
    /// @return Current timestamp as 100-nanosecond ticks since Unix epoch
    /// @note Thread-safe: Can be called concurrently from multiple threads
    int64_t get_timestamp_ticks() noexcept {
        const auto NOW = std::chrono::system_clock::now();
        const auto DURATION = NOW.time_since_epoch();

        const auto TICKS = std::chrono::duration_cast<std::chrono::duration<int64_t, std::ratio<1, TICKS_PER_SECOND>>>(DURATION);

        return TICKS.count();
    }
} // namespace visus::cuid2::utils
