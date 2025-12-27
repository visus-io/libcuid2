/// @file platform.hpp
/// @brief Cross-platform abstraction layer for system-specific operations
///
/// Provides unified interfaces for platform-dependent functionality required by
/// CUID2 generation. Implementations handle both Windows (MSVC/MinGW) and POSIX
/// (Linux/macOS/FreeBSD/OpenBSD/NetBSD) systems.

#ifndef LIBCUID2_PLATFORM_HPP
#define LIBCUID2_PLATFORM_HPP

#include <map>
#include <string>

namespace visus::cuid2::platform {
    /// Fills a buffer with cryptographically secure random bytes.
    ///
    /// Uses OpenSSL's RAND_bytes() CSPRNG for cryptographic-quality randomness.
    ///
    /// @param buf Pointer to buffer to fill with random bytes
    /// @param LEN Number of random bytes to generate
    /// @note Thread-safe: Can be called concurrently from multiple threads
    void get_random_bytes(void *buf, size_t LEN) noexcept;

    /// Generates a cryptographically secure random 64-bit integer.
    ///
    /// @return A cryptographically random int64_t value
    /// @note Thread-safe: Can be called concurrently from multiple threads
    [[nodiscard]] int64_t get_random_int64() noexcept;

    /// Retrieves the system hostname.
    ///
    /// Platform-specific implementation:
    /// - Windows: Uses GetComputerNameA()
    /// - POSIX: Uses gethostname()
    ///
    /// Falls back to a random hex string if retrieval fails.
    ///
    /// @return The system hostname, or a random hex string on failure
    [[nodiscard]] std::string get_hostname();

    /// Returns the current process ID.
    ///
    /// Platform-specific implementation:
    /// - Windows: Uses GetCurrentProcessId()
    /// - POSIX: Uses getpid()
    ///
    /// @return The current process ID
    /// @note Thread-safe: Process ID is constant for the lifetime of the process
    [[nodiscard]] int get_process_id() noexcept;

    /// Retrieves all environment variables as key-value pairs.
    ///
    /// Platform-specific implementation:
    /// - Windows: Uses GetEnvironmentStringsW() with UTF-16 to UTF-8 conversion
    /// - POSIX: Uses global environ variable
    ///
    /// @return A map of environment variable names to values (automatically sorted by key)
    /// @note The map is sorted for deterministic fingerprint generation
    [[nodiscard]] std::map<std::string, std::string> get_environment_variables();

}

#endif //LIBCUID2_PLATFORM_HPP