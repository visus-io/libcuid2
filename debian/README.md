# Debian Packaging for libcuid2

This directory contains the Debian packaging files for libcuid2, a C++ implementation of Cuid2 collision-resistant unique identifiers.

## Package Structure

The source package builds three binary packages:

1. **libcuid2-0** - Shared library package
   - Contains the runtime shared library (`libcuid2.so.0`)
   - Multi-arch: same (can be installed for multiple architectures)

2. **libcuid2-dev** - Development package
   - Contains header files (`/usr/include/cuid2/*.hpp`)
   - CMake configuration files (`/usr/lib/*/cmake/cuid2/`)
   - Development symlink (`libcuid2.so`)
   - Library API and architecture manual pages (section 3 and 7)

3. **cuid2gen** - Command-line tool
   - CLI executable for generating Cuid2 identifiers
   - User manual page (section 1)

## Building the Package

### Prerequisites

Install build dependencies:

```bash
sudo apt-get install debhelper-compat cmake pkg-config \
    libssl-dev libboost-dev libfmt-dev
```

### Build Process

From the project root directory:

```bash
# Option 1: Build unsigned packages for local testing
dpkg-buildpackage -us -uc -b

# Option 2: Build source and binary packages
dpkg-buildpackage -us -uc

# Option 3: Using debuild
debuild -us -uc -b
```

The built packages will be created in the parent directory:
- `libcuid2-0_0.0.1-1_<arch>.deb`
- `libcuid2-dev_0.0.1-1_<arch>.deb`
- `cuid2gen_0.0.1-1_<arch>.deb`

### Installing Locally

```bash
sudo dpkg -i ../libcuid2-0_*.deb ../libcuid2-dev_*.deb ../cuid2gen_*.deb
sudo apt-get install -f  # Install any missing dependencies
```

## Package Testing

### Lintian Checks

Check package quality:

```bash
lintian -i -I --show-overrides ../libcuid2-0_*.deb
lintian -i -I --show-overrides ../libcuid2-dev_*.deb
lintian -i -I --show-overrides ../cuid2gen_*.deb
```

### Verify Package Contents

```bash
# List files in each package
dpkg-deb -c ../libcuid2-0_*.deb
dpkg-deb -c ../libcuid2-dev_*.deb
dpkg-deb -c ../cuid2gen_*.deb

# Show package metadata
dpkg-deb -I ../libcuid2-0_*.deb
```

### Test Installation

```bash
# Create a clean test environment (optional)
debootstrap unstable /tmp/debian-test
sudo chroot /tmp/debian-test

# Or use schroot/pbuilder for isolated testing
```

## Dependencies

### Build Dependencies

- **debhelper-compat (= 13)** - Debian build helper tools
- **cmake (>= 4.0)** - Build system
- **pkg-config** - Dependency detection
- **libssl-dev (>= 3.0)** - OpenSSL development files (SHA3-512, CSPRNG)
- **libboost-dev (>= 1.74)** - Boost libraries (header-only components)
- **libfmt-dev (>= 9.0)** - Modern C++ formatting library

### Runtime Dependencies

Automatically determined by `${shlibs:Depends}`:
- libssl3 (or appropriate OpenSSL version)
- libstdc++6
- libc6

## Package Configuration

### CMake Build Options

The `debian/rules` file configures the build with:

- `CMAKE_BUILD_TYPE=Release` - Optimized release build
- `BUILD_SHARED_LIBS=ON` - Build shared library
- `BUILD_TESTS=ON` - Enable unit tests (run during build)
- `ENABLE_SANITIZERS=OFF` - Disable sanitizers for production
- `ENABLE_COVERAGE=OFF` - Disable coverage instrumentation

### Hardening

Full hardening enabled via `DEB_BUILD_MAINT_OPTIONS = hardening=+all`:
- Stack protector (`-fstack-protector-strong`)
- Fortify source (`-D_FORTIFY_SOURCE=2`)
- Format string checks
- Read-only relocations (`RELRO`)
- Immediate binding (`BIND_NOW`)

## Multiarch Support

The library packages are marked as `Multi-Arch: same`, allowing:
- Installation of multiple architectures simultaneously
- Cross-compilation support
- Proper co-installability (e.g., amd64 + i386)

## Manual Pages

Manual pages are automatically installed:

- **cuid2gen(1)** - CLI tool user manual → `/usr/share/man/man1/`
- **libcuid2(3)** - Library API reference → `/usr/share/man/man3/`
- **libcuid2(7)** - Architecture overview → `/usr/share/man/man7/`

Localized versions (12 languages) are also installed to `/usr/share/man/<locale>/`.

## Updating the Package

### Version Bumps

Update version in:
1. `debian/changelog` - Add new entry with `dch -i` or manually
2. `CMakeLists.txt` - Update `project(libcuid2 VERSION x.y.z)`
3. `vcpkg.json` - Update `"version": "x.y.z"`

### Adding Changelog Entries

```bash
# Interactive changelog editing
dch -i

# Or manually follow Debian changelog format:
# package (version) distribution; urgency=level
#   * Change description
#  -- Maintainer <email>  Date in RFC 2822 format
```

## Submitting to Debian

Before submitting to Debian:

1. **ITP (Intent to Package)** - File a bug report against `wnpp` package
2. **Mentors** - Upload to mentors.debian.net for review
3. **Sponsor** - Find a Debian Developer sponsor
4. **Policy Compliance** - Ensure Debian Policy 4.6.2 compliance
5. **FTP Masters** - Package review and upload to NEW queue

See: https://www.debian.org/devel/wnpp/

## Useful Commands

```bash
# Clean build artifacts
debian/rules clean
# Or
dh clean

# Build only architecture-specific packages
dpkg-buildpackage -B

# Build only architecture-independent packages (none in this package)
dpkg-buildpackage -A

# Sign packages
debsign ../*.changes

# Check for outdated build dependencies
dose-builddebcheck
```

## License

The packaging files are licensed under the MIT License, same as the upstream source.

## PPA (Personal Package Archive) for Ubuntu

The debian packaging files are configured to support building for Ubuntu PPAs on Launchpad.

### Supported Ubuntu Releases

Source packages are pre-configured for the following Ubuntu LTS releases:

- **Ubuntu 24.04 LTS (Noble Numbat)** - Version suffix: `~noble1`
- **Ubuntu 22.04 LTS (Jammy Jellyfish)** - Version suffix: `~jammy1`
- **Ubuntu 20.04 LTS (Focal Fossa)** - Version suffix: `~focal1`

### PPA Prerequisites

Install required tools:

```bash
sudo apt-get install devscripts debhelper git git-buildpackage dput
```

### GPG Key Setup

PPAs require signed source packages. Set up GPG if needed:

```bash
# Generate GPG key (if you don't have one)
gpg --full-generate-key

# List your keys
gpg --list-secret-keys --keyid-format LONG

# Upload public key to Ubuntu keyserver
gpg --keyserver keyserver.ubuntu.com --send-keys YOUR_KEY_ID
```

### Building for PPA

#### Quick Build (Automated Script)

Use the provided build script for all releases:

```bash
cd /path/to/libcuid2

# Build unsigned packages (sign later)
./debian/build-ppa.sh

# Build and sign with your GPG key
./debian/build-ppa.sh -k YOUR_GPG_KEY_ID
```

The script will:
1. Create the orig tarball from git
2. Build source packages for Noble, Jammy, and Focal
3. Place all files in `../build-area/`

#### Manual Build Process

For a single Ubuntu release:

```bash
# 1. Create orig tarball
git archive --format=tar --prefix=libcuid2-0.0.1/ HEAD | xz -9 > ../libcuid2_0.0.1.orig.tar.xz

# 2. Build source package for Noble (24.04)
debuild -S -sa -k<YOUR_GPG_KEY_ID>

# 3. Verify generated files
ls -l ../*noble1*
```

For additional releases, update debian/changelog and rebuild:

```bash
# Build for Jammy
debuild -S -sa -k<YOUR_GPG_KEY_ID>

# Build for Focal
debuild -S -sa -k<YOUR_GPG_KEY_ID>
```

### Uploading to Launchpad PPA

#### First-Time PPA Setup

1. Create a PPA on Launchpad: https://launchpad.net/~your-username/+activate-ppa
2. Configure dput:

```bash
cat >> ~/.dput.cf << 'EOF'
[libcuid2-ppa]
fqdn = ppa.launchpad.net
method = ftp
incoming = ~YOUR_LAUNCHPAD_USERNAME/ubuntu/libcuid2/
login = anonymous
allow_unsigned_uploads = 0
EOF
```

#### Upload Source Packages

```bash
# Upload to your PPA (replace with your PPA name)
dput ppa:YOUR_LAUNCHPAD_USERNAME/libcuid2 ../build-area/libcuid2_*~noble1_source.changes
dput ppa:YOUR_LAUNCHPAD_USERNAME/libcuid2 ../build-area/libcuid2_*~jammy1_source.changes
dput ppa:YOUR_LAUNCHPAD_USERNAME/libcuid2 ../build-area/libcuid2_*~focal1_source.changes
```

Launchpad will automatically:
- Build the packages for multiple architectures (amd64, arm64, etc.)
- Publish to your PPA
- Send you email notifications

### Using the PPA

Once published, users can install from your PPA:

```bash
# Add PPA
sudo add-apt-repository ppa:YOUR_LAUNCHPAD_USERNAME/libcuid2
sudo apt-get update

# Install packages
sudo apt-get install libcuid2-0 libcuid2-dev cuid2gen

# Use the library
cuid2gen                    # Generate default length ID
cuid2gen -l 32              # Generate 32-character ID
```

### Signing Packages After Building

If you built unsigned packages, sign them before uploading:

```bash
# Sign all changes files
debsign -k<YOUR_GPG_KEY_ID> ../build-area/libcuid2_*_source.changes

# Or sign individually
debsign -k<YOUR_GPG_KEY_ID> ../build-area/libcuid2_0.0.1-1ubuntu1~noble1_source.changes
```

### Testing PPA Builds Locally

Before uploading, test building in clean environments:

```bash
# Install pbuilder
sudo apt-get install pbuilder ubuntu-dev-tools

# Create base environments
pbuilder-dist noble create
pbuilder-dist jammy create
pbuilder-dist focal create

# Test build
pbuilder-dist noble build ../build-area/libcuid2_*~noble1.dsc
pbuilder-dist jammy build ../build-area/libcuid2_*~jammy1.dsc
pbuilder-dist focal build ../build-area/libcuid2_*~focal1.dsc
```

### Version Management for PPAs

Version format: `UPSTREAM_VERSION-DEBIAN_REVISION~ubuntuX~CODENAME1`

Example: `0.0.1-1ubuntu1~noble1`

- `0.0.1` - Upstream version
- `-1` - Debian revision
- `ubuntu1` - Ubuntu revision
- `~noble1` - Release-specific revision (tilde ensures upgrade path)

When updating:

```bash
# New upstream release
dch -v 0.0.2-1ubuntu1~noble1 "New upstream release"

# Bug fix for existing version
dch -v 0.0.1-1ubuntu2~noble1 "Fix build on Noble"

# Update across releases
dch -v 0.0.1-1ubuntu1~jammy2 "Rebuild for Jammy dependencies"
```

### Troubleshooting PPA Builds

**Build failures:**
1. Check Launchpad build logs: https://launchpad.net/~YOUR_USERNAME/+archive/ubuntu/libcuid2/+builds
2. Verify dependencies are available in target Ubuntu release
3. Test locally with pbuilder first

**GPG errors:**
```bash
# Ensure key is on Ubuntu keyserver
gpg --keyserver keyserver.ubuntu.com --send-keys YOUR_KEY_ID

# Wait a few minutes for propagation
# Retry upload
```

**Dependency issues:**
- Check that all Build-Depends exist in the target Ubuntu release
- Adjust version requirements if needed for older releases
- Consider backporting dependencies if necessary

### PPA Configuration Files

- `debian/changelog` - Multi-release version entries
- `debian/control` - Package metadata and dependencies
- `debian/source/options` - Source package options and exclusions
- `debian/gbp.conf` - Git-buildpackage configuration
- `debian/build-ppa.sh` - Automated build script
- `debian/watch` - Upstream version monitoring

## Resources

- [Debian Policy Manual](https://www.debian.org/doc/debian-policy/)
- [Debian New Maintainers' Guide](https://www.debian.org/doc/manuals/maint-guide/)
- [Debhelper Documentation](https://man7.org/linux/man-pages/man7/debhelper.7.html)
- [CMake Debian Integration](https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html)
- [Ubuntu PPA Documentation](https://help.launchpad.net/Packaging/PPA)
- [Launchpad Build Farm](https://launchpad.net/builders)
