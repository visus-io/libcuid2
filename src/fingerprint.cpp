/// @file fingerprint.cpp
/// @brief System fingerprint computation for CUID2 generation
///
/// This file implements a singleton fingerprint that uniquely identifies the
/// current system and process. The fingerprint combines hostname, process ID,
/// and environment variables into a deterministic byte sequence that remains
/// constant for the lifetime of the process.

#include "cuid2/fingerprint.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <string>
#include <vector>

#include "cuid2/platform.hpp"
#include <boost/endian/conversion.hpp>

namespace visus::cuid2 {
    namespace {
        /// Generates the system fingerprint byte sequence.
        ///
        /// Creates a unique fingerprint by concatenating:
        /// 1. Hostname (string bytes)
        /// 2. Process ID (4 bytes, little-endian uint32)
        /// 3. Environment variables (sorted key=value pairs)
        ///
        /// The process ID is serialized in little-endian format using
        /// boost::endian::native_to_little() for cross-platform compatibility.
        /// Environment variables are pre-sorted by the platform layer to ensure
        /// deterministic output.
        ///
        /// @return A byte vector containing the concatenated fingerprint data
        std::vector<uint8_t> generate() {
            std::string hostname = platform::get_hostname();
            const int PROCESS_ID = platform::get_process_id();

            const auto env_vars = platform::get_environment_variables();

            size_t env_size = 0;
            for (const auto& [key, value] : env_vars) {
                env_size += key.size() + 1 + value.size();
            }

            std::string env_string;
            env_string.reserve(env_size);

            for (const auto& [key, value] : env_vars) {
                env_string += key;
                env_string += '=';
                env_string += value;
            }

            const size_t TOTAL_SIZE = hostname.size() + sizeof(uint32_t) + env_string.size();

            std::vector<uint8_t> result;
            result.reserve(TOTAL_SIZE);

            std::ranges::copy(hostname, std::back_inserter(result));

            const auto PID = boost::endian::native_to_little(static_cast<uint32_t>(PROCESS_ID));
            const auto PID_BYTES = std::bit_cast<std::array<uint8_t, sizeof(uint32_t)>>(PID);

            std::ranges::copy(PID_BYTES, std::back_inserter(result));

            std::ranges::copy(env_string, std::back_inserter(result));

            return result;
        }
    }

    /// Constructs the Fingerprint singleton and computes the system fingerprint.
    ///
    /// The fingerprint is computed once during construction by calling generate()
    /// and cached for the lifetime of the process. This ensures consistent
    /// fingerprint values across all CUID2 generations within the same process.
    ///
    /// @note This constructor is private and only called once during static initialization
    Fingerprint::Fingerprint() : cached_value_{generate()} {
    }

    /// Returns the cached system fingerprint.
    ///
    /// This static method accesses the inline static singleton instance and returns
    /// a reference to the pre-computed fingerprint byte sequence. The fingerprint
    /// remains constant for the lifetime of the process.
    ///
    /// @return A const reference to the fingerprint byte vector
    /// @note Thread-safe: Can be called concurrently from multiple threads
    const std::vector<uint8_t>& Fingerprint::get() {
        return instance_.cached_value_;
    }
} // namespace visus::cuid2
