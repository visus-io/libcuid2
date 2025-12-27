# vcpkg Overlay Ports for System Libraries

This directory contains vcpkg overlay ports that redirect dependency resolution to system-installed libraries on non-Windows platforms.

## Purpose

On Unix-like platforms (macOS, Linux, FreeBSD, OpenBSD, NetBSD), these overlay ports tell vcpkg to:
1. Skip downloading and building dependencies
2. Verify system libraries are installed
3. Fail with helpful error messages if libraries are missing

On Windows, these overlays are not used—Windows builds use vcpkg's standard ports.

## Directory Structure

Each subdirectory represents a stub port:

- `openssl/` - System OpenSSL (libssl-dev, openssl@3, etc.)
- `fmt/` - System fmt library (libfmt-dev, fmt, etc.) - *Falls back to vcpkg if not found*
- `boost-endian/` - System Boost.Endian (header-only)
- `boost-multiprecision/` - System Boost.Multiprecision (header-only)
- `boost-nowide/` - System Boost.Nowide (header-only)
- `boost-test/` - System Boost.Test / unit_test_framework

## How It Works

When CMake is configured with `VCPKG_OVERLAY_PORTS` pointing to this directory:

1. vcpkg checks overlay ports before checking its official registry
2. Each `portfile.cmake` runs `find_package()` to verify the system library exists
3. If found: Sets `VCPKG_POLICY_EMPTY_PACKAGE` to skip installation
4. If not found: Fails with platform-specific installation instructions (or falls back to vcpkg for fmt)

## Installation Requirements

Before building, install these system libraries:

### macOS (Homebrew)
```bash
brew install openssl@3 boost fmt
```

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libssl-dev libboost-all-dev libfmt-dev
```

### Fedora/RHEL
```bash
sudo dnf install openssl-devel boost-devel fmt-devel
```

### Arch Linux
```bash
sudo pacman -S openssl boost fmt
```

### FreeBSD
```bash
pkg install openssl boost-all libfmt
```

### OpenBSD
```bash
pkg_add openssl boost fmt
```

### NetBSD
```bash
pkgin install openssl boost fmt
```

## Modifying Overlay Ports

To add a new system library dependency:

1. Create a directory: `cmake/vcpkg/ports/<package-name>/`
2. Add `vcpkg.json` with package metadata:
   ```json
   {
     "name": "<package-name>",
     "version": "<approximate-version>",
     "description": "System-provided <library> (stub port for non-Windows platforms)"
   }
   ```
3. Add `portfile.cmake` with system library detection logic:
   ```cmake
   if(VCPKG_TARGET_IS_WINDOWS)
       message(FATAL_ERROR "This overlay port is for non-Windows platforms only.")
   endif()

   message(STATUS "Checking for system-installed <LibraryName>...")
   find_package(<PackageName> QUIET [CONFIG] [COMPONENTS ...])

   if(NOT <PackageName>_FOUND)
       message(FATAL_ERROR
           "<LibraryName> not found on system. Please install:\n"
           "  macOS:          brew install <package>\n"
           "  Ubuntu/Debian:  sudo apt-get install <package>-dev\n"
           "  ...")
   endif()

   message(STATUS "Found system <LibraryName>: ${<PackageName>_VERSION}")
   set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
   ```

## Troubleshooting

### "library not found" errors during configuration

**Problem**: CMake reports that OpenSSL, Boost, or fmt cannot be found.

**Solution**: Install the missing library using your system package manager (see Installation Requirements above).

### fmt falls back to vcpkg build

**Behavior**: You see a warning "fmt not found on system. Falling back to vcpkg build..."

**Explanation**: The fmt overlay port uses a hybrid approach - it prefers system libraries but falls back to vcpkg if not found. This is intentional since fmt is not available in all package managers.

**To use system fmt**: Install fmt via your package manager, then reconfigure.

### Windows build fails with overlay port error

**Problem**: Windows build fails with "This overlay port is for non-Windows platforms only."

**Cause**: The `default-windows` preset in CMakePresets.json is incorrectly configured or the wrong preset is being used.

**Solution**: Verify you're using a Windows preset (`windows-x64-debug`, `windows-x64-release`, etc.) which should inherit from `default-windows`, not `default-unix`.

### Version mismatch warnings

**Problem**: System library version differs from what vcpkg would provide.

**Impact**: Generally harmless for stable libraries (OpenSSL 3.x, Boost 1.75+, fmt 8+), but may cause issues with very old system libraries.

**Solution**: Upgrade system libraries or modify overlay port `portfile.cmake` to add version checks:
```cmake
if(<Library>_FOUND AND <Library>_VERSION VERSION_LESS "minimum.version")
    message(FATAL_ERROR "Version ${<Library>_VERSION} found, but minimum.version+ required")
endif()
```

## References

- [vcpkg Overlay Ports Documentation](https://learn.microsoft.com/en-us/vcpkg/concepts/overlay-ports)
- [Using System Package Manager Dependencies with vcpkg](https://devblogs.microsoft.com/cppblog/using-system-package-manager-dependencies-with-vcpkg/)
- [vcpkg Policies Reference](https://learn.microsoft.com/en-us/vcpkg/reference/policies)
