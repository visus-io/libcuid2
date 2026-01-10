# libcuid2

[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/visus-io/libcuid2/ci.yml?style=for-the-badge&logo=github)](https://github.com/visus-io/libcuid2/actions/workflows/ci.yaml)
[![Sonar Quality Gate](https://img.shields.io/sonar/quality_gate/visus%3Alibcuid2?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge&logo=sonarcloud&logoColor=white)](https://sonarcloud.io/summary/overall?id=visus%3Alibcuid2)
[![Sonar Coverage](https://img.shields.io/sonar/coverage/visus%3Alibcuid2?server=https%3A%2F%2Fsonarcloud.io&style=for-the-badge&logo=sonarcloud&logoColor=white)](https://sonarcloud.io/summary/overall?id=visus%3Alibcuid2)
![GitHub License](https://img.shields.io/github/license/visus-io/libcuid2?style=for-the-badge)

A high-performance C++ implementation of the [Cuid2](https://github.com/paralleldrive/cuid2) specification for generating collision-resistant, sortable unique identifiers using cryptographic primitives.

<details>
<summary><strong>Table of Contents</strong></summary>

- [libcuid2](#libcuid2)
  - [Features](#features)
  - [Quick Start](#quick-start)
  - [Installation](#installation)
    - [CMake Installation](#cmake-installation)
      - [Checking Your CMake Version](#checking-your-cmake-version)
      - [Ubuntu/Debian](#ubuntudebian)
      - [Fedora/RHEL](#fedorarhel)
      - [Arch Linux](#arch-linux)
      - [macOS](#macos)
      - [FreeBSD](#freebsd)
      - [OpenBSD/NetBSD](#openbsdnetbsd)
      - [Windows](#windows)
    - [System Dependencies](#system-dependencies)
      - [macOS](#macos-1)
      - [Ubuntu/Debian](#ubuntudebian-1)
      - [Fedora/RHEL](#fedorarhel-1)
      - [Arch Linux](#arch-linux-1)
      - [FreeBSD](#freebsd-1)
      - [Windows](#windows-1)
    - [vcpkg Setup](#vcpkg-setup)
    - [Build from Source](#build-from-source)
      - [CMake Presets](#cmake-presets)
    - [Debian/Ubuntu Packages](#debianubuntu-packages)
  - [Usage](#usage)
    - [Command-Line Tool](#command-line-tool)
    - [Library API](#library-api)
      - [Basic Usage](#basic-usage)
      - [Error Handling](#error-handling)
      - [Thread-Safe Concurrent Generation](#thread-safe-concurrent-generation)
    - [CMake Integration](#cmake-integration)
  - [Algorithm](#algorithm)
  - [Documentation](#documentation)
    - [Manual Pages](#manual-pages)
    - [Internationalization](#internationalization)
    - [API Documentation](#api-documentation)
  - [Testing](#testing)
    - [Test Coverage](#test-coverage)
  - [Platform Support](#platform-support)
  - [Requirements](#requirements)
  - [Architecture](#architecture)
    - [Platform Abstraction Layer](#platform-abstraction-layer)
    - [Cryptography](#cryptography)
    - [Thread Safety](#thread-safety)
  - [Contributing](#contributing)
    - [Development Workflow](#development-workflow)
    - [Code Quality Standards](#code-quality-standards)
  - [Additional Documentation](#additional-documentation)

</details>

## Features

- **Collision-Resistant**: Uses NIST FIPS-202 SHA3-512 hashing and cryptographically secure random number generation
- **Sortable**: Contains timestamp component for chronological ordering
- **URL-Safe**: Base-36 encoded (lowercase alphanumeric)
- **Configurable Length**: 4-32 characters (default: 24)
- **Thread-Safe**: Atomic counter with comprehensive concurrent access testing
- **Cross-Platform**: Windows (MSVC/MinGW), Linux, macOS, FreeBSD, OpenBSD, NetBSD
- **Standards-Compliant**: C++20, NIST FIPS-202, POSIX
- **Well-Documented**: Doxygen API docs + Unix manpages
- **Security-Hardened**: Stack protection, fortify source, control flow integrity

## Quick Start

```cpp
#include <cuid2/cuid2.hpp>
#include <iostream>

int main() {
    // Generate default length (24 characters)
    std::string id = cuid2::generate_cuid2();
    std::cout << id << std::endl;
    // Output: c9k2l3m4n5o6p7q8r9s0t1u2

    // Generate custom length
    std::string short_id = cuid2::generate_cuid2(16);
    std::cout << short_id << std::endl;
    // Output: a1b2c3d4e5f6g7h8

    return 0;
}
```

## Installation

### CMake Installation

This project requires **CMake 3.22+**. Most modern Linux distributions ship with CMake 3.22 or newer by default.

#### Checking Your CMake Version
```bash
cmake --version
```

#### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install cmake
```

Ubuntu 22.04 LTS and later include CMake 3.22+. For older versions, use the Kitware APT repository:

```bash
# Add Kitware APT repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
sudo apt-get update
sudo apt-get install cmake
```

#### Fedora/RHEL

```bash
sudo dnf install cmake
```

#### Arch Linux
```bash
sudo pacman -S cmake
```

#### macOS
```bash
brew install cmake
```

#### FreeBSD
```bash
pkg install cmake
```

#### OpenBSD/NetBSD
```bash
pkg_add cmake      # OpenBSD
pkgin install cmake # NetBSD
```

#### Windows

**Option 1: Official Installer**
- Download from https://cmake.org/download/
- Run the installer and add CMake to system PATH

**Option 2: Chocolatey**
```powershell
choco install cmake
```

**Option 3: Winget**
```powershell
winget install Kitware.CMake
```

### System Dependencies

#### macOS
```bash
brew install openssl@3 boost fmt
```

#### Ubuntu/Debian
```bash
sudo apt-get install libssl-dev libboost-all-dev libfmt-dev
```

#### Fedora/RHEL
```bash
sudo dnf install openssl-devel boost-devel fmt-devel
```

#### Arch Linux
```bash
sudo pacman -S openssl boost fmt
```

#### FreeBSD
```bash
pkg install openssl boost-all libfmt
```

#### Windows
No system package installation required - vcpkg will build all dependencies automatically (see vcpkg Setup below).

### vcpkg Setup

The project uses [vcpkg](https://github.com/microsoft/vcpkg) for dependency management with manifest mode (`vcpkg.json`). Dependencies are automatically installed during CMake configuration.

**Installation:**
```bash
# Clone vcpkg
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh  # or bootstrap-vcpkg.bat on Windows

# Set environment variable
export VCPKG_ROOT=/path/to/vcpkg  # Add to your shell profile
```

**How it works:**
- **Non-Windows platforms** (macOS, Linux, BSD): Uses overlay ports to redirect to system-installed libraries (see System Dependencies above)
- **Windows**: Automatically builds all dependencies via vcpkg's standard ports

**Windows/Visual Studio users:**
```bash
# Optional: Integrate vcpkg system-wide with Visual Studio
vcpkg integrate install
```
This makes vcpkg packages available to all Visual Studio projects without manually specifying the toolchain file.

The CMake presets expect `VCPKG_ROOT` to be set. Alternatively, specify the toolchain file manually:

```bash
cmake --preset macos-arm64-release \
  -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

For detailed information about the overlay ports system, see [cmake/vcpkg/ports/README.md](cmake/vcpkg/ports/README.md).

### Build from Source

```bash
# Configure (adjust platform/architecture as needed)
cmake --preset macos-arm64-release

# Build
cmake --build --preset macos-arm64-release

# Install (optional)
sudo cmake --install build-macos-arm64-release
```

#### CMake Presets

Available presets for common platforms:
```bash
# List all available presets
cmake --list-presets

# Examples:
cmake --preset linux-x64-debug
cmake --preset windows-x64-release
cmake --preset freebsd-arm64-debug
```

### Debian/Ubuntu Packages

**Note:** Building DEB packages requires CMake 3.22+. See [CMake Installation](#cmake-installation) above if you don't have it installed.

Build DEB packages:
```bash
# Install build dependencies
sudo apt-get install debhelper-compat pkg-config \
    libssl-dev libboost-dev libfmt-dev

# Build packages
dpkg-buildpackage -us -uc -b

# Install packages
sudo dpkg -i ../libcuid2-0_*.deb ../libcuid2-dev_*.deb ../cuid2gen_*.deb
```

Three packages are built:
- `libcuid2-0` - Runtime shared library
- `libcuid2-dev` - Development headers and CMake config
- `cuid2gen` - Command-line tool

## Usage

### Command-Line Tool

```bash
# Generate default length (24)
cuid2gen
# Output: c9k2l3m4n5o6p7q8r9s0t1u2

# Generate custom length
cuid2gen -l 16
# Output: a1b2c3d4e5f6g7h8

# Use in shell scripts
USER_ID=$(cuid2gen)
echo "Created user: $USER_ID"
```

### Library API

#### Basic Usage

```cpp
#include <cuid2/cuid2.hpp>

// Generate default length (24)
std::string id = cuid2::generate_cuid2();

// Generate custom length (4-32)
std::string short_id = cuid2::generate_cuid2(16);
```

#### Error Handling

```cpp
#include <cuid2/cuid2.hpp>
#include <iostream>

try {
    std::string id = cuid2::generate_cuid2(64); // Invalid length
} catch (const std::invalid_argument& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

#### Thread-Safe Concurrent Generation

```cpp
#include <cuid2/cuid2.hpp>
#include <thread>
#include <vector>

void generate_ids(std::vector<std::string>& ids, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        ids.push_back(cuid2::generate_cuid2());
    }
}

int main() {
    constexpr size_t num_threads = 10;
    constexpr size_t ids_per_thread = 1000;

    std::vector<std::thread> threads;
    std::vector<std::vector<std::string>> results(num_threads);

    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back(generate_ids, std::ref(results[i]), ids_per_thread);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // All IDs are guaranteed unique
    return 0;
}
```

### CMake Integration

```cmake
find_package(cuid2 REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE cuid2::cuid2)
```

## Algorithm

Cuid2 combines five components for uniqueness:

1. **Random Prefix** (a-z) - Ensures valid identifiers
2. **Timestamp** (Unix epoch) - Enables sortability
3. **Counter** (atomic, thread-safe) - Prevents collisions in rapid generation
4. **Fingerprint** (hostname + PID + environment) - System uniqueness
5. **Random Bytes** (CSPRNG) - Cryptographic collision resistance

All components are hashed with NIST FIPS-202 SHA3-512 and encoded as base-36.

## Documentation

### Manual Pages

After installation, access documentation via `man`:
```bash
man cuid2gen          # CLI tool reference
man 3 libcuid2        # API reference with examples
man 7 libcuid2        # Architecture and design
```

### Internationalization

Manpages currently available in English. The project includes localization infrastructure ready for translations into 11 additional languages (German, Spanish, French, Italian, Japanese, Korean, Polish, Portuguese (European), Portuguese (Brazilian), Russian, Chinese (Simplified)).

Translation contributions are welcome! See [man/LOCALIZATION.md](man/LOCALIZATION.md) for guidelines.

### API Documentation

Full Doxygen documentation available in source:
- Headers: `include/cuid2/*.hpp`
- Implementation: `src/*.cpp`

## Testing

```bash
# Build with tests
cmake --preset macos-arm64-debug
cmake --build --preset macos-arm64-debug

# Run all tests
ctest --preset macos-arm64-debug

# Run specific test suites
./build-macos-arm64-debug/cuid2_test
./build-macos-arm64-debug/counter_test
./build-macos-arm64-debug/fingerprint_test
```

### Test Coverage

37 test cases across 5 suites:
- **cuid2_test** (15 tests) - Generation, validation, uniqueness, thread safety
- **counter_test** (3 tests) - Thread safety, atomic increments
- **fingerprint_test** (6 tests) - Deterministic structure, singleton behavior
- **utils_test** (7 tests) - Base-36 encoding, prefix generation
- **platform_test** (6 tests) - Cross-platform abstractions

## Platform Support

| Platform | Architectures |
|----------|--------------|
| **Linux** | x86_64, ARM64 |
| **macOS** | x86_64, ARM64 (Apple Silicon) |
| **Windows** | x86_64, ARM64 (MSVC/MinGW) |
| **FreeBSD** | x86_64, ARM64 |
| **OpenBSD** | x86_64, ARM64 |
| **NetBSD** | x86_64, ARM64 |

## Requirements

- **C++20** compiler (GCC 10+, Clang 11+, MSVC 2019+)
- **CMake** 3.22+
- **OpenSSL** 3.x (NIST FIPS-202 SHA3-512, CSPRNG)
- **Boost** (Endian, Multiprecision, Nowide, Test)
- **fmt** (Modern formatting library)

## Architecture

### Platform Abstraction Layer

Single unified implementation (`src/platform.cpp`) with preprocessor directives:
- **Windows**: `GetComputerNameA()`, `GetCurrentProcessId()`, UTF-16 conversion
- **POSIX** (Linux/macOS/BSD): `gethostname()`, `getpid()`, `environ`
- **CSPRNG**: OpenSSL `RAND_bytes()` (cross-platform)

### Cryptography

- **Hashing**: NIST FIPS-202 SHA3-512 via OpenSSL EVP interface
- **Random**: `RAND_bytes()` (FIPS 140-3 validated)
- **Encoding**: Base-36 using Boost.Multiprecision arbitrary precision integers

### Thread Safety

- **Counter**: `std::atomic<int64_t>` with `.fetch_add()`
- **Fingerprint**: Singleton pattern (C++11+ thread-safe static initialization)
- Extensively tested with 10-20 concurrent threads generating up to 50,000 IDs

## Contributing

Contributions welcome! Please follow these guidelines:

### Development Workflow

1. Fork the repository
2. Install system dependencies (see Installation section)
3. Create a feature branch
4. Make changes following existing code style
5. Run tests: `ctest --preset <platform>-<arch>-debug`
6. Update documentation if adding features or changing behavior
7. Submit a pull request with clear description of changes

### Code Quality Standards

- Follow C++20 best practices
- Maintain Doxygen documentation for all public APIs
- Ensure cross-platform compatibility (Windows, Linux, macOS, BSD)
- Add test coverage for new functionality
- Pass all existing tests and linters

## Additional Documentation

- [debian/README.md](debian/README.md) - Debian packaging documentation
- [man/README.md](man/README.md) - Manual pages overview
- [man/LOCALIZATION.md](man/LOCALIZATION.md) - Translation guidelines
- [cmake/vcpkg/ports/README.md](cmake/vcpkg/ports/README.md) - vcpkg overlay ports information
