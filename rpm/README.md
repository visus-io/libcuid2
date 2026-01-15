# RPM Packaging for libcuid2

This directory contains the RPM spec file for building libcuid2 packages on Red Hat-based distributions.

## Supported Distributions

- **RHEL 9** and derivatives (Rocky Linux 9, AlmaLinux 9, Oracle Linux 9)
- **RHEL 8** and derivatives (Rocky Linux 8, AlmaLinux 8, Oracle Linux 8)
- **CentOS Stream 8/9**
- **Fedora** (current supported versions)

## Package Structure

The spec file produces three packages:

| Package | Description |
|---------|-------------|
| `libcuid2` | Shared library (`libcuid2.so.0`) |
| `libcuid2-devel` | Development files (headers, CMake config, man pages) |
| `cuid2gen` | Command-line tool for generating identifiers |

## Prerequisites

### Fedora

```bash
sudo dnf install rpm-build rpmdevtools gcc-c++ cmake \
    openssl-devel boost-devel fmt-devel
```

### RHEL 9 / Rocky Linux 9 / AlmaLinux 9

```bash
sudo dnf install rpm-build rpmdevtools gcc-c++ cmake \
    openssl-devel boost-devel fmt-devel

# Enable EPEL for additional dependencies if needed
sudo dnf install epel-release
```

### RHEL 8 / Rocky Linux 8 / AlmaLinux 8

RHEL 8 requires GCC Toolset for C++20 support:

```bash
# Enable EPEL for OpenSSL 3.x and fmt
sudo dnf install epel-release

# Install build dependencies
sudo dnf install gcc-toolset-12-gcc-c++ cmake3 \
    rpm-build rpmdevtools openssl3-devel boost-devel fmt-devel

# Enable GCC Toolset before building
source /opt/rh/gcc-toolset-12/enable
```

## Building RPM Packages

### Method 1: Using rpmbuild (Standard)

```bash
# Set up RPM build environment
rpmdev-setuptree

# Copy spec file
cp libcuid2.spec ~/rpmbuild/SPECS/

# Download or create source tarball
# Option A: Download from GitHub (when released)
spectool -g -R ~/rpmbuild/SPECS/libcuid2.spec

# Option B: Create tarball from local source
cd /path/to/libcuid2
git archive --format=tar.gz --prefix=libcuid2-1.0.1/ \
    -o ~/rpmbuild/SOURCES/libcuid2-1.0.1.tar.gz HEAD

# Build packages
rpmbuild -ba ~/rpmbuild/SPECS/libcuid2.spec

# Built packages will be in:
# ~/rpmbuild/RPMS/<arch>/libcuid2-1.0.1-1.<dist>.<arch>.rpm
# ~/rpmbuild/RPMS/<arch>/libcuid2-devel-1.0.1-1.<dist>.<arch>.rpm
# ~/rpmbuild/RPMS/<arch>/cuid2gen-1.0.1-1.<dist>.<arch>.rpm
# ~/rpmbuild/SRPMS/libcuid2-1.0.1-1.<dist>.src.rpm
```

### Method 2: Using mock (Clean Build Environment)

Mock builds packages in a clean chroot, ensuring reproducibility:

```bash
# Install mock
sudo dnf install mock

# Add user to mock group
sudo usermod -a -G mock $USER
newgrp mock

# Create source RPM first
rpmbuild -bs ~/rpmbuild/SPECS/libcuid2.spec

# Build for specific target (e.g., Fedora 41)
mock -r fedora-41-x86_64 ~/rpmbuild/SRPMS/libcuid2-1.0.1-1.fc41.src.rpm

# Build for RHEL 9
mock -r rocky-9-x86_64 ~/rpmbuild/SRPMS/libcuid2-1.0.1-1.el9.src.rpm

# Build for RHEL 8
mock -r rocky-8-x86_64 ~/rpmbuild/SRPMS/libcuid2-1.0.1-1.el8.src.rpm
```

### Method 3: Using Copr (Cloud Build Service)

For automated builds across multiple distributions:

```bash
# Install copr-cli
sudo dnf install copr-cli

# Create a Copr project (one-time setup)
copr-cli create libcuid2 --chroot fedora-41-x86_64 \
    --chroot fedora-40-x86_64 --chroot epel-9-x86_64 --chroot epel-8-x86_64

# Upload source RPM
copr-cli build libcuid2 ~/rpmbuild/SRPMS/libcuid2-1.0.1-1.*.src.rpm
```

## Installing Built Packages

```bash
# Install runtime library
sudo dnf install ./libcuid2-1.0.1-1.*.rpm

# Install CLI tool
sudo dnf install ./cuid2gen-1.0.1-1.*.rpm

# Install development files
sudo dnf install ./libcuid2-devel-1.0.1-1.*.rpm
```

## Verification

After installation, verify the packages:

```bash
# Test CLI tool
cuid2gen
cuid2gen --length 16

# View man pages
man cuid2gen
man 3 libcuid2
man 7 libcuid2

# Check library
ldconfig -p | grep cuid2

# Compile a test program
cat > test_cuid2.cpp << 'EOF'
#include <cuid2/cuid2.hpp>
#include <iostream>

int main() {
    std::cout << cuid2::generate() << std::endl;
    return 0;
}
EOF

g++ -std=c++20 test_cuid2.cpp -lcuid2 -o test_cuid2
./test_cuid2
```

## Package Dependencies

### Runtime Dependencies

| Package | libcuid2 | cuid2gen |
|---------|----------|----------|
| openssl-libs >= 3.0 | Yes | (via libcuid2) |
| libstdc++ | Yes | Yes |

### Development Dependencies

| Package | Required For |
|---------|--------------|
| openssl-devel >= 3.0 | Linking |
| boost-devel >= 1.74 | Header-only components |

## Distribution-Specific Notes

### RHEL 8 / CentOS Stream 8

- **C++20 Support**: Requires GCC Toolset 12 (`gcc-toolset-12-gcc-c++`)
- **CMake**: Use `cmake3` package (cmake in RHEL 8 is version 3.20)
- **OpenSSL 3.x**: Available via EPEL (`openssl3-devel`)
- **fmt**: Available via EPEL (`fmt-devel`)

### RHEL 9 / CentOS Stream 9

- Ships with GCC 11 (full C++20 support)
- CMake 3.26+ available
- OpenSSL 3.0 included in base repositories

### Fedora

- Latest GCC and CMake versions
- All dependencies available in base repositories

## Troubleshooting

### CMake version too old

```bash
# RHEL 8: Use cmake3
sudo dnf install cmake3
# Spec file handles this automatically
```

### GCC doesn't support C++20

```bash
# RHEL 8: Enable GCC Toolset
source /opt/rh/gcc-toolset-12/enable
```

### Missing fmt-devel

```bash
# Enable EPEL repository
sudo dnf install epel-release
sudo dnf install fmt-devel
```

## License

MIT License - see LICENSE file in the project root.
