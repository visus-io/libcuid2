# ==============================================================================
# libcuid2 RPM Spec File
# ==============================================================================

%global soversion 0

Name:           libcuid2
Version:        1.0.1
Release:        1%{?dist}
Summary:        Collision-resistant unique identifier generation library

License:        MIT
URL:            https://github.com/visus-io/libcuid2
Source0:        %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

# Common build requirements
BuildRequires:  boost-devel >= 1.74
BuildRequires:  fmt-devel >= 8.1

# RHEL 8 specific build requirements (requires EPEL)
%if 0%{?rhel} == 8
BuildRequires:  cmake3 >= 3.22
BuildRequires:  gcc-toolset-12-gcc-c++
BuildRequires:  openssl3-devel
%else
# RHEL 9+, Fedora, CentOS Stream 9+
BuildRequires:  cmake >= 3.22
BuildRequires:  gcc-c++ >= 11
BuildRequires:  openssl-devel >= 3.0
%endif

%description
libcuid2 is a C++ implementation of Cuid2, providing secure, collision-resistant,
URL-safe, and sortable unique identifiers.

Features:
  * Collision-resistant using cryptographic primitives (SHA3-512)
  * Sortable by timestamp for chronological ordering
  * URL-safe base-36 encoding
  * Thread-safe concurrent generation
  * Configurable length (4-32 characters, default 24)

# ==============================================================================
# Main Package: libcuid2 (shared library)
# ==============================================================================
# Runtime dependency: OpenSSL 3.x (different package name on RHEL 8)
%if 0%{?rhel} == 8
Requires:       openssl3-libs
%else
Requires:       openssl-libs >= 3.0
%endif

# ==============================================================================
# Subpackage: libcuid2-devel (development files)
# ==============================================================================
%package devel
Summary:        Development files for libcuid2
Requires:       %{name}%{?_isa} = %{version}-%{release}
Requires:       boost-devel >= 1.74
%if 0%{?rhel} == 8
Requires:       openssl3-devel
%else
Requires:       openssl-devel >= 3.0
%endif

%description devel
libcuid2 is a C++ implementation of Cuid2, providing secure, collision-resistant,
URL-safe, and sortable unique identifiers.

This package contains the development files including headers, CMake
configuration files, and manual pages for library functions.

# ==============================================================================
# Subpackage: cuid2gen (CLI tool)
# ==============================================================================
%package -n cuid2gen
Summary:        Command-line tool for generating Cuid2 identifiers
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description -n cuid2gen
cuid2gen is a command-line utility for generating Cuid2 identifiers.

Cuid2 identifiers are collision-resistant, sortable, URL-safe unique
identifiers suitable for distributed systems, databases, and web applications.

This package contains the command-line executable and user manual.

# ==============================================================================
# Build
# ==============================================================================
%prep
%autosetup -n %{name}-%{version}

%build
%if 0%{?rhel} == 8
# Enable GCC Toolset 12 for C++20 support on RHEL 8
source /opt/rh/gcc-toolset-12/enable
# Use cmake3 macro on RHEL 8
%cmake3 \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON
%cmake3_build
%else
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON
%cmake_build
%endif

%check
%if 0%{?rhel} == 8
%ctest3
%else
%ctest
%endif

%install
%if 0%{?rhel} == 8
%cmake3_install
%else
%cmake_install
%endif

# ==============================================================================
# Scriptlets
# ==============================================================================
%ldconfig_scriptlets

# ==============================================================================
# Files
# ==============================================================================
%files
%license LICENSE
%doc README.md
%{_libdir}/libcuid2.so.%{soversion}
%{_libdir}/libcuid2.so.%{version}

%files devel
%{_includedir}/cuid2/
%{_libdir}/libcuid2.so
%{_libdir}/cmake/cuid2/
%{_mandir}/man3/libcuid2.3*
%{_mandir}/man7/libcuid2.7*

%files -n cuid2gen
%{_bindir}/cuid2gen
%{_mandir}/man1/cuid2gen.1*

# ==============================================================================
# Changelog
# ==============================================================================
%changelog
* Wed Jan 15 2026 Visus Development Team <admin@projects.visus.io> - 1.0.1-1
- New upstream release 1.0.1
- C++ implementation of Cuid2 identifier generation
- Thread-safe collision-resistant unique identifiers
- SHA3-512 cryptographic hashing (NIST FIPS-202)
- Configurable length (4-32 characters, default 24)
- Cross-platform support (Linux, BSD, macOS, Windows)
