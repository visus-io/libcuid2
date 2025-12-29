/// @file cuid2.cpp
/// @brief Main CUID2 identifier generation implementation
///
/// This file implements the core CUID2 generation algorithm which combines:
/// - Timestamp (for sortability)
/// - Atomic counter (for uniqueness within same timestamp)
/// - System fingerprint (for uniqueness across processes/machines)
/// - Cryptographic random bytes (for collision resistance)
/// - NIST FIPS-202 SHA3-512 hashing (for output uniformity)
/// - Base-36 encoding (for compact, URL-safe representation)
///
/// The resulting identifiers are collision-resistant, sortable by creation time,
/// and safe for use in URLs and as database identifiers.

#include "cuid2/cuid2.hpp"

#include <array>
#include <bit>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string_view>

#include <boost/endian/conversion.hpp>
#include <openssl/evp.h>

#include "cuid2/counter.hpp"
#include "cuid2/fingerprint.hpp"
#include "cuid2/platform.hpp"
#include "cuid2/utils.hpp"

#include <fmt/core.h>

namespace visus::cuid2 {
    namespace {
        /// Combined size of timestamp and counter in bytes (8 bytes each).
        constexpr size_t TIMESTAMP_COUNTER_SIZE = sizeof(int64_t) * 2;

        /// Length of the random letter prefix (always 1 character).
        constexpr size_t PREFIX_LENGTH = 1;

        /// Serializes a 64-bit integer to little-endian bytes and appends to buffer.
        ///
        /// Converts the input value to unsigned, then to little-endian byte order
        /// using boost::endian for cross-platform compatibility. The 8 bytes are
        /// appended to the buffer in little-endian order.
        ///
        /// @param buffer Vector to append the serialized bytes to
        /// @param VALUE The 64-bit integer to serialize
        [[gnu::noinline]] void serialize_int64_le(std::vector<uint8_t>& buffer, const int64_t VALUE) {
            const auto UNSIGNED_VALUE = static_cast<uint64_t>(VALUE);
            const auto LITTLE_ENDIAN_VALUE = boost::endian::native_to_little(UNSIGNED_VALUE);
            const auto BYTES = std::bit_cast<std::array<uint8_t, sizeof(uint64_t)>>(LITTLE_ENDIAN_VALUE);

            buffer.insert(buffer.end(), BYTES.begin(), BYTES.end());
        }

        /// Validates the requested CUID2 length is within allowed bounds.
        ///
        /// Checks that the requested length is between MIN_CUID2_LENGTH (4) and
        /// MAX_CUID2_LENGTH (32) inclusive. Shorter identifiers have higher
        /// collision probability; longer ones provide more entropy.
        ///
        /// @param MAX_LENGTH The requested CUID2 identifier length
        /// @throws std::invalid_argument if length is outside valid range
        void validate_length(const int MAX_LENGTH) {
            if (MAX_LENGTH < MIN_CUID2_LENGTH || MAX_LENGTH > MAX_CUID2_LENGTH) [[unlikely]] {
                throw std::invalid_argument(
                    fmt::format("MAX_LENGTH must be between {} and {}",
                                MIN_CUID2_LENGTH, MAX_CUID2_LENGTH)
                );
            }
        }

        /// Builds the input buffer for SHA3-512 hashing.
        ///
        /// Concatenates all CUID2 components in a specific order:
        /// 1. Timestamp (8 bytes, little-endian)
        /// 2. Counter (8 bytes, little-endian)
        /// 3. Fingerprint (variable length)
        /// 4. Random bytes (variable length)
        ///
        /// This deterministic ordering ensures consistent hash outputs for testing
        /// and cross-implementation compatibility.
        ///
        /// @param TIMESTAMP Current timestamp in 100-nanosecond ticks
        /// @param COUNTER Current counter value
        /// @param fingerprint System fingerprint bytes
        /// @param random_bytes Cryptographically secure random bytes
        /// @return Concatenated byte vector ready for hashing
        [[nodiscard]] std::vector<uint8_t> build_hash_input(
            const int64_t TIMESTAMP,
            const int64_t COUNTER,
            const std::vector<uint8_t>& fingerprint,
            const std::vector<uint8_t>& random_bytes
        ) {
            std::vector<uint8_t> hash_input;
            hash_input.reserve(TIMESTAMP_COUNTER_SIZE + fingerprint.size() + random_bytes.size());

            serialize_int64_le(hash_input, TIMESTAMP);
            serialize_int64_le(hash_input, COUNTER);
            hash_input.insert(hash_input.end(), fingerprint.begin(), fingerprint.end());
            hash_input.insert(hash_input.end(), random_bytes.begin(), random_bytes.end());

            return hash_input;
        }

        /// Computes NIST FIPS-202 SHA3-512 hash of the input bytes.
        ///
        /// Uses OpenSSL's EVP interface to compute SHA3-512 (the standardized
        /// NIST FIPS-202 variant, not the original Keccak). This provides a
        /// 64-byte (512-bit) cryptographic hash with strong collision resistance.
        ///
        /// @param input Byte vector to hash
        /// @return 64-byte SHA3-512 hash output
        /// @throws Cuid2Error if OpenSSL operations fail
        [[nodiscard]] std::vector<uint8_t> compute_hash(const std::vector<uint8_t>& input) {
            /// RAII deleter for OpenSSL EVP_MD_CTX context.
            /// Ensures EVP_MD_CTX_free() is called when the unique_ptr goes out of scope.
            struct EVPContextDeleter {
                void operator()(EVP_MD_CTX* ctx) const noexcept {
                    EVP_MD_CTX_free(ctx);
                }
            };

            const std::unique_ptr<EVP_MD_CTX, EVPContextDeleter> CTX(EVP_MD_CTX_new());
            if (!CTX) [[unlikely]] {
                throw Cuid2Error("Failed to create SHA-3(512) hash context");
            }

            if (EVP_DigestInit_ex(CTX.get(), EVP_sha3_512(), nullptr) != 1) {
                throw Cuid2Error("Failed to initialize SHA-3(512) hash");
            }

            if (EVP_DigestUpdate(CTX.get(), input.data(), input.size()) != 1) {
                throw Cuid2Error("Failed to update SHA-3(512) hash");
            }

            std::vector<uint8_t> hash_output(EVP_MD_size(EVP_sha3_512()));

            if (unsigned int hash_len = 0; EVP_DigestFinal_ex(CTX.get(), hash_output.data(), &hash_len) != 1) {
                throw Cuid2Error("Failed to finalize SHA-3(512) hash");
            }

            return hash_output;
        }

        /// Formats the final CUID2 identifier by combining prefix and encoded hash.
        ///
        /// Constructs the final identifier as: [prefix][encoded_hash_substring]
        /// The result is truncated to MAX_LENGTH characters total. The prefix is
        /// always 1 character (a-z), so the encoded hash contributes (MAX_LENGTH - 1)
        /// characters.
        ///
        /// @param PREFIX Random lowercase letter prefix (a-z)
        /// @param ENCODED Base-36 encoded hash string
        /// @param MAX_LENGTH Total desired identifier length (including prefix)
        /// @return Formatted CUID2 identifier of exact length MAX_LENGTH
        [[nodiscard]] std::string format_result(const char PREFIX, const std::string_view ENCODED, const int MAX_LENGTH) {
            std::string result;
            result.reserve(MAX_LENGTH);
            result += PREFIX;

            if (const size_t NEEDED_LENGTH = MAX_LENGTH - PREFIX_LENGTH; ENCODED.length() >= NEEDED_LENGTH) [[likely]] {
                result += ENCODED.substr(0, NEEDED_LENGTH);
            } else {
                result += ENCODED;
            }

            return result;
        }
    } // anonymous namespace

    /// Generates a CUID2 identifier of the specified length.
    ///
    /// Creates a collision-resistant, sortable unique identifier by combining:
    /// 1. Current timestamp (for sortability and uniqueness over time)
    /// 2. Atomic counter (for uniqueness within same timestamp)
    /// 3. System fingerprint (for uniqueness across processes/machines)
    /// 4. Cryptographic random bytes (for collision resistance)
    /// 5. Random prefix character (ensures valid identifier in most languages)
    ///
    /// All components are hashed with SHA3-512 and encoded as base-36 for a
    /// compact, URL-safe representation.
    ///
    /// @param MAX_LENGTH Desired identifier length (default: 24, min: 4, max: 32)
    /// @return A CUID2 identifier string of exact length MAX_LENGTH
    /// @throws std::invalid_argument if MAX_LENGTH is outside valid range [4, 32]
    /// @throws Cuid2Error if cryptographic operations fail (extremely rare)
    /// @note Thread-safe: Can be called concurrently from multiple threads
    std::string generate(const int MAX_LENGTH) {
        validate_length(MAX_LENGTH);

        const int64_t TIMESTAMP = utils::get_timestamp_ticks();
        const int64_t COUNTER = Counter::next();
        const auto& fingerprint = Fingerprint::get();

        std::vector<uint8_t> random_bytes(MAX_LENGTH);
        platform::get_random_bytes(random_bytes.data(), MAX_LENGTH);

        const char PREFIX = utils::generate_prefix();

        const auto HASH_INPUT = build_hash_input(TIMESTAMP, COUNTER, fingerprint, random_bytes);
        const auto HASH_OUTPUT = compute_hash(HASH_INPUT);
        const auto ENCODED = utils::encode_base36(HASH_OUTPUT);

        return format_result(PREFIX, ENCODED, MAX_LENGTH);
    }
} // namespace visus::cuid2