# GitHub Copilot Instructions for libcuid2

This file provides guidance to GitHub Copilot when suggesting code for the libcuid2 project.

## Project Overview

**libcuid2** is a high-performance C++ implementation of the Cuid2 specification for generating collision-resistant, sortable unique identifiers using cryptographic primitives. The project is **feature complete** with comprehensive cross-platform support.

## Code Standards

### Language & Compiler Requirements

- **C++20 standard** (required)
- **CMake 3.22+** for build system
- Compiler support: GCC 10+, Clang 11+, MSVC 2019+, MinGW64
- Use modern C++ features and idioms

### Coding Style

- Follow existing code style in the repository
- Use snake_case for functions and variables
- Use PascalCase for class names
- Namespace: All public APIs under `cuid2::`
- Header guards: Use `#pragma once`
- Line length: Keep reasonable (no strict limit, but aim for readability)

### Documentation Standards

All code must be documented using **Doxygen-style documentation**:

```cpp
/// @file
/// @brief Brief description of the file

/// @brief Function description
/// @param param_name Parameter description
/// @return Return value description
/// @throws exception_type When and why this exception is thrown
/// @note Additional notes about thread safety, performance, etc.
```

**Key documentation principles:**
- Use `///` triple-slash comments for Doxygen
- Document *what* and *why*, not *how*
- **No inline documentation within function bodies** (keep implementation clean)
- File-level, class-level, and function-level documentation required for public APIs
- Explicitly document thread safety where relevant

## Project Structure

```
libcuid2/
├── include/cuid2/       # Public headers
│   ├── cuid2.hpp        # Main API
│   ├── counter.hpp      # Atomic counter
│   ├── fingerprint.hpp  # System fingerprint
│   ├── platform.hpp     # Cross-platform abstractions
│   └── utils.hpp        # Utility functions
├── src/                 # Implementation files
│   ├── cuid2.cpp        # Main generation logic
│   ├── counter.cpp      # Thread-safe counter
│   ├── fingerprint.cpp  # Fingerprint computation
│   ├── platform.cpp     # Platform-specific code (Windows/POSIX)
│   ├── utils.cpp        # Base-36 encoding, timestamps
│   └── cuid2gen.cpp     # CLI executable
├── tests/               # Unit tests
└── cmake/vcpkg/ports/   # vcpkg overlay ports (system libs)
```

## Build System

### CMake Conventions

- Build directory naming: `build-{os}-{arch}-{config}`
  - OS: `macos`, `linux`, `freebsd`, `openbsd`, `netbsd`, `windows`
  - Arch: `arm64`, `x64`
  - Config: `debug`, `release`
- Example: `build-macos-arm64-debug`, `build-linux-x64-release`

### CMake Presets

Use CMake presets for configuration:
```bash
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug
```

## Dependencies

The project uses **vcpkg** for dependency management with overlay ports:

### Core Dependencies

- **OpenSSL 3.x** - NIST FIPS-202 SHA3-512 hashing, CSPRNG
  - Use `EVP_sha3_512()` for hashing
  - Use `RAND_bytes()` for cryptographically secure random numbers
  - Link via: `OpenSSL::Crypto` target

- **Boost** (header-only components)
  - `boost::endian::native_to_little()` for endianness conversion
  - `boost::multiprecision::cpp_int` for base-36 encoding
  - `boost::nowide::narrow()` for Windows UTF-16 to UTF-8 conversion

- **fmt** - Modern formatting library
  - Use `fmt::format()` and `fmt::print()` instead of iostream where appropriate

### Dependency Strategy

- **Non-Windows** (macOS, Linux, BSD): Uses system-installed libraries via vcpkg overlay ports
- **Windows**: vcpkg builds all dependencies automatically

## Platform Abstraction

### Cross-Platform Code

Platform-specific code lives in `src/platform.cpp` using preprocessor directives:

```cpp
#ifdef _WIN32
    // Windows implementation (MSVC/MinGW)
    // GetComputerNameA(), GetCurrentProcessId(), GetEnvironmentStringsW()
#else
    // POSIX implementation (Linux/macOS/BSD)
    // gethostname(), getpid(), environ
#endif
```

### Platform Functions

- `generate_random_bytes()` - OpenSSL `RAND_bytes()` (cross-platform)
- `get_process_id()` - Process ID retrieval
- `get_hostname()` - Hostname with fallback to random hex string
- `get_environment_variables()` - Sorted environment map

## Critical Implementation Details

### Endianness Handling

**MUST** serialize timestamp and counter in little-endian format for cross-platform compatibility:

```cpp
#include <boost/endian/conversion.hpp>

// Use boost::endian for conversion
int64_t value = /* ... */;
int64_t le_value = boost::endian::native_to_little(value);
```

### Environment Variable Ordering

Environment variables **MUST** be sorted alphabetically by key before hashing:

```cpp
// Use std::map which automatically maintains sorted order
std::map<std::string, std::string> env_vars;
```

### Thread Safety

- Counter: Use `std::atomic<int64_t>` with `.fetch_add()`
- Fingerprint: Singleton pattern (C++11+ thread-safe static initialization)
- No manual locking required for these components

### SHA-3 Variant

**CRITICAL**: Use NIST FIPS-202 SHA3-512 (not original Keccak):

```cpp
#include <openssl/evp.h>

EVP_MD_CTX* ctx = EVP_MD_CTX_new();
EVP_DigestInit_ex(ctx, EVP_sha3_512(), nullptr);  // NIST FIPS-202 variant
EVP_DigestUpdate(ctx, data, length);

std::vector<uint8_t> hash(EVP_MD_size(EVP_sha3_512()));
unsigned int hash_len = 0;
EVP_DigestFinal_ex(ctx, hash.data(), &hash_len);
EVP_MD_CTX_free(ctx);
```

## Security Guidelines

### Compiler Security Flags

The build system enables:
- Stack protection: `-fstack-protector-strong` (GCC/Clang) or `/GS` (MSVC)
- Fortify source: `-D_FORTIFY_SOURCE=2` (Linux/macOS)
- Control flow integrity: `-fcf-protection=full` (x86_64 GCC 8+/Clang 8+)

### Security Best Practices

- Always use OpenSSL's `RAND_bytes()` for cryptographic randomness
- Never use `rand()` or `<random>` for security-critical values
- Validate all input parameters (length bounds: 4-32 for Cuid2)
- Use `std::atomic` for thread-safe counter operations

## Testing

### Test Framework

- Uses **Boost.Test** framework
- Test files: `tests/*_test.cpp`
- Current coverage: 37 test cases across 5 suites

### Writing Tests

```cpp
#define BOOST_TEST_MODULE YourModuleName
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_case_name) {
    // Arrange
    // Act
    // Assert
    BOOST_REQUIRE(condition);
    BOOST_CHECK_EQUAL(expected, actual);
}
```

### Running Tests

```bash
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug
ctest --preset macos-arm64-debug
```

## API Design Principles

### Public API

The main public API is minimal and focused:

```cpp
namespace cuid2 {
    /// Default Cuid2 length
    constexpr size_t DEFAULT_LENGTH = 24;

    /// Minimum Cuid2 length
    constexpr size_t MIN_CUID2_LENGTH = 4;

    /// Maximum Cuid2 length
    constexpr size_t MAX_CUID2_LENGTH = 32;

    /// Generate a Cuid2 identifier
    /// @throws std::invalid_argument if length is out of bounds
    std::string generate_cuid2(size_t length = DEFAULT_LENGTH);
}
```

### Error Handling

- Use exceptions for error conditions: `std::invalid_argument`, `std::runtime_error`
- Provide clear error messages
- Document all exceptions in Doxygen comments

### Symbol Visibility

Use `CUID2_API` macro for exported symbols:

```cpp
#include <cuid2/cuid2_export.hpp>

CUID2_API std::string generate_cuid2(size_t length = DEFAULT_LENGTH);
```

## Performance Considerations

- Base-36 encoding uses `boost::multiprecision::cpp_int` for arbitrary precision
- Counter uses atomic operations (no locks)
- Fingerprint computed once and cached (singleton)
- OpenSSL operations are optimized and FIPS-validated

## Common Patterns

### Base-36 Encoding

```cpp
#include <boost/multiprecision/cpp_int.hpp>

std::string encode_base36(const std::vector<uint8_t>& bytes) {
    boost::multiprecision::cpp_int num;
    // Convert bytes to big integer
    // Divide by 36 repeatedly, collecting remainders
    // Map remainders to [0-9a-z]
}
```

### Little-Endian Serialization

```cpp
#include <boost/endian/conversion.hpp>
#include <bit>
#include <array>

std::array<uint8_t, 8> serialize_int64_le(int64_t value) {
    int64_t le_value = boost::endian::native_to_little(value);
    return std::bit_cast<std::array<uint8_t, 8>>(le_value);
}
```

## Additional Resources

- **CLAUDE.md** - Comprehensive project documentation for Claude Code
- **README.md** - Public-facing documentation and usage examples
- **man/** - Unix manual pages (CLI tool, API reference, architecture)
- **debian/README.md** - Debian packaging documentation
- **cmake/vcpkg/ports/README.md** - vcpkg overlay ports information

## What NOT to Do

- ❌ Don't add inline comments within function bodies (keep implementation clean)
- ❌ Don't use `rand()` or `<random>` for cryptographic randomness
- ❌ Don't use platform-specific APIs outside `src/platform.cpp`
- ❌ Don't add dependencies without discussion (project is feature complete)
- ❌ Don't ignore endianness (always use little-endian serialization)
- ❌ Don't use original Keccak (use NIST FIPS-202 SHA3-512)
- ❌ Don't create files unless absolutely necessary
- ❌ Don't modify existing file behavior without comprehensive testing

## Interoperability

For compatibility with other Cuid2 implementations:
- Use NIST FIPS-202 SHA3-512 (domain separation byte 0x06, not 0x01)
- Little-endian serialization for timestamp/counter
- Same base-36 encoding algorithm
- Matching fingerprint logic (hostname + PID + sorted environment variables)

## License

All code contributions must be compatible with the MIT license. Dependencies use MIT-compatible licenses (Apache 2.0, BSL 1.0).
