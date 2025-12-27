/// @file platform.cpp
/// @brief Cross-platform abstraction layer for system-specific operations
///
/// This file provides unified interfaces for platform-dependent functionality
/// required by CUID2 generation. It uses preprocessor directives to select
/// between Windows (MSVC/MinGW) and POSIX (Linux/macOS/BSD) implementations.
///
/// Key abstractions:
/// - Cryptographically secure random number generation (OpenSSL)
/// - Hostname retrieval with fallback to random generation
/// - Process ID retrieval
/// - Environment variable enumeration with automatic UTF-8 conversion

#include "cuid2/platform.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <functional>
#include <stdexcept>

#include <openssl/rand.h>

#include <fmt/core.h>

#ifdef _WIN32
    #include <memory>
    #include <windows.h> // NOSONAR(S3806) - Microsoft uses lowercase windows.h
    #include <processenv.h>
    #include <boost/nowide/convert.hpp>
#else
    #include <cstring>

    #include <unistd.h>
extern char **environ; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
#endif

namespace visus::cuid2::platform {
    namespace {
        /// Generates a random hostname fallback as a hexadecimal string.
        ///
        /// Used when system hostname retrieval fails. Generates 8 random bytes
        /// from the CSPRNG and formats them as a 16-character hexadecimal string.
        ///
        /// @return A random hex string suitable as a hostname fallback
        auto generate_random_hostname() -> std::string {
            constexpr size_t RANDOM_BYTES = 8;
            std::array<unsigned char, RANDOM_BYTES> random_bytes{};

            get_random_bytes(random_bytes.data(), random_bytes.size());

            std::string result;
            result.reserve(RANDOM_BYTES * 2);
            for (const auto BYTE : random_bytes) {
                fmt::format_to(std::back_inserter(result), "{:02x}", BYTE);
            }

            return result;
        }
    } // anonymous namespace

    /// Fills a buffer with cryptographically secure random bytes.
    ///
    /// Uses OpenSSL's RAND_bytes() which provides a CSPRNG suitable for
    /// cryptographic operations. OpenSSL 3.x automatically initializes
    /// the random number generator on first use.
    ///
    /// @param buf Pointer to buffer to fill with random bytes
    /// @param LEN Number of random bytes to generate
    /// @note Thread-safe: Can be called concurrently from multiple threads
    /// @note No explicit initialization required (OpenSSL 3.x auto-initializes)
    void get_random_bytes(unsigned char *buf, const size_t LEN) noexcept {
        RAND_bytes(buf, static_cast<int>(LEN));
    }

    /// Generates a cryptographically secure random 64-bit integer.
    ///
    /// Convenience wrapper around get_random_bytes() for generating random
    /// int64_t values using OpenSSL's CSPRNG. Uses std::bit_cast for
    /// type-safe conversion from bytes to int64_t.
    ///
    /// @return A cryptographically random int64_t value
    /// @note Thread-safe: Can be called concurrently from multiple threads
    int64_t get_random_int64() noexcept {
        std::array<unsigned char, sizeof(int64_t)> bytes{};
        RAND_bytes(bytes.data(), bytes.size());
        return std::bit_cast<int64_t>(bytes);
    }

    /// Returns the current process ID.
    ///
    /// Platform-specific implementation:
    /// - Windows: Uses GetCurrentProcessId()
    /// - POSIX (Linux/macOS/BSD): Uses getpid()
    ///
    /// @return The current process ID as an integer
    /// @note Thread-safe: Process ID is constant for the lifetime of the process
    int get_process_id() noexcept {
#ifdef _WIN32
        return static_cast<int>(GetCurrentProcessId());
#else
        return getpid();
#endif
    }

#ifdef _WIN32
    // ============================================================================
    // Windows Implementation (MSVC/MinGW)
    // ============================================================================

    /// Buffer size for Windows hostname retrieval.
    /// MAX_COMPUTERNAME_LENGTH is defined by Windows.h (typically 15 characters).
    constexpr DWORD HOSTNAME_BUFFER_SIZE = MAX_COMPUTERNAME_LENGTH + 1;

    /// Retrieves the Windows computer name as the hostname.
    ///
    /// Uses GetComputerNameA() to retrieve the NetBIOS name of the local computer.
    /// If retrieval fails, falls back to generate_random_hostname() to ensure
    /// fingerprint generation can continue.
    ///
    /// @return The computer name, or a random hex string if retrieval fails
    std::string get_hostname() {
        std::array<char, HOSTNAME_BUFFER_SIZE> hostname{};

        if (auto size = static_cast<DWORD>(hostname.size()); GetComputerNameA(hostname.data(), &size)) {
            return hostname.data();
        }

        return generate_random_hostname();
    }

    /// Retrieves all environment variables as key-value pairs (Windows).
    ///
    /// Uses GetEnvironmentStringsW() to retrieve the environment block as
    /// UTF-16 strings, then converts each key-value pair to UTF-8 using
    /// boost::nowide. The returned map is automatically sorted by key
    /// using std::less<> for lexicographical ordering.
    ///
    /// @return A map of environment variable names to values (UTF-8 encoded)
    /// @note The map is automatically sorted by key for deterministic fingerprints
    std::map<std::string, std::string, std::less<>> get_environment_variables() {
        std::map<std::string, std::string, std::less<>> env_vars;

        /// RAII deleter for Windows environment strings block.
        /// Ensures FreeEnvironmentStringsW() is called when the unique_ptr goes out of scope.
        struct EnvStringsDeleter {
            void operator()(LPWCH ptr) const noexcept {
                if (ptr != nullptr) {
                    FreeEnvironmentStringsW(ptr);
                }
            }
        };

        const std::unique_ptr<WCHAR, EnvStringsDeleter> ENV_BLOCK(GetEnvironmentStringsW());
        if (ENV_BLOCK == nullptr) {
            return env_vars;
        }

        for (LPWCH env = ENV_BLOCK.get(); *env != L'\0';) {
            LPCWCH delimiter = nullptr;
            size_t length = 0;
            for (LPCWCH ptr = env; *ptr != L'\0'; ++ptr, ++length) {
                if (*ptr == L'=' && delimiter == nullptr) {
                    delimiter = ptr;
                }
            }

            if (delimiter != nullptr) [[likely]] {
                const size_t KEY_LENGTH = delimiter - env;

                env_vars.try_emplace(
                    boost::nowide::narrow(env, KEY_LENGTH),
                    boost::nowide::narrow(delimiter + 1));
            }

            env += length + 1;
        }

        return env_vars;
    }

#else
    // ============================================================================
    // POSIX Implementation (Linux/macOS/FreeBSD/OpenBSD/NetBSD)
    // ============================================================================

    /// Buffer size for POSIX hostname retrieval.
    /// POSIX.1 doesn't specify a maximum hostname length, but 256 bytes is
    /// a common convention and sufficient for most systems.
    constexpr size_t HOSTNAME_BUFFER_SIZE = 256;

    /// Retrieves the system hostname via gethostname().
    ///
    /// Uses the POSIX gethostname() function to retrieve the hostname.
    /// If retrieval fails (returns non-zero), falls back to
    /// generate_random_hostname() to ensure fingerprint generation can continue.
    ///
    /// @return The system hostname, or a random hex string if retrieval fails
    std::string get_hostname() {
        std::array<char, HOSTNAME_BUFFER_SIZE> hostname{};

        if (gethostname(hostname.data(), hostname.size()) == 0) {
            return hostname.data();
        }

        return generate_random_hostname();
    }

    /// Retrieves all environment variables as key-value pairs (POSIX).
    ///
    /// Iterates over the global `environ` variable to extract all environment
    /// variables. Each entry is parsed as "KEY=VALUE" and stored in a std::map.
    /// The returned map is automatically sorted by key using std::less<>
    /// for lexicographical ordering.
    ///
    /// @return A map of environment variable names to values
    /// @note The map is automatically sorted by key for deterministic fingerprints
    std::map<std::string, std::string, std::less<>> get_environment_variables() {
        std::map<std::string, std::string, std::less<>> env_vars;

        for (const char * const *env = environ; *env != nullptr; env++) {
            if (const char *delimiter = std::strchr(*env, '='); delimiter != nullptr) [[likely]] {
                const size_t KEY_LENGTH = delimiter - *env;

                env_vars.try_emplace(
                    std::string(*env, KEY_LENGTH),
                    std::string(delimiter + 1));
            }
        }

        return env_vars;
    }

#endif

} // namespace visus::cuid2::platform
