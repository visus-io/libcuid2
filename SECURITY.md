# Security Policy

## Supported Versions

We release patches for security vulnerabilities in the following versions:

| Version | Supported          |
| ------- | ------------------ |
| 1.0.x   | :white_check_mark: |
| < 1.0   | :x:                |

## Security Update Policy

Security updates are released as soon as possible after a vulnerability is confirmed and a fix is available. Critical vulnerabilities receive immediate attention and expedited releases.

## Reporting a Vulnerability

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, please report security vulnerabilities via email to:

**security@projects.visus.io**

### What to Include

Please include the following information in your report:

1. **Type of vulnerability** (e.g., buffer overflow, cryptographic weakness, timing attack)
2. **Full paths of affected source files**
3. **Location of affected code** (tag/branch/commit or direct URL)
4. **Step-by-step instructions to reproduce the issue**
5. **Proof-of-concept or exploit code** (if available)
6. **Impact assessment** (what an attacker could achieve)
7. **Suggested mitigation** (if you have one)

### Response Timeline

You can expect the following response timeline:

- **Initial Response**: Within 48 hours acknowledging receipt
- **Assessment**: Within 7 days with initial assessment and severity classification
- **Status Updates**: Regular updates every 7-14 days until resolution
- **Resolution**: Timeline depends on severity
  - Critical: 7-14 days
  - High: 14-30 days
  - Medium: 30-60 days
  - Low: 60-90 days

### Disclosure Policy

We follow coordinated disclosure:

1. Security issues are handled confidentially until a fix is released
2. We work with reporters to understand and validate the issue
3. Fixes are developed and tested privately
4. Security advisories are published when fixes are released
5. Credit is given to reporters (unless anonymity is requested)

## Security Scope

### In Scope

The following security concerns are in scope for this project:

1. **Cryptographic Issues**
   - Weaknesses in SHA3-512 implementation or usage
   - CSPRNG (cryptographically secure random number generator) failures
   - Entropy weaknesses affecting uniqueness guarantees

2. **ID Generation Vulnerabilities**
   - Predictability of generated identifiers
   - Collision vulnerabilities beyond theoretical probability
   - Timing attacks that could reveal system information

3. **Memory Safety**
   - Buffer overflows, use-after-free, memory leaks
   - Undefined behavior in C++ code
   - Thread safety violations in concurrent usage

4. **Dependency Vulnerabilities**
   - Known CVEs in OpenSSL, Boost, or fmt
   - Supply chain attacks via vcpkg or build system

5. **Platform-Specific Issues**
   - Platform abstraction layer vulnerabilities
   - Privilege escalation through system calls
   - Information disclosure via environment variables or system fingerprinting

### Out of Scope

The following are generally not considered security vulnerabilities:

1. **Theoretical Collision Probability**
   - The documented collision probability for default 24-character IDs (see libcuid2.7 manpage)
   - Collisions are statistically possible but extremely unlikely with proper usage

2. **Deployment Configuration**
   - Misuse of the library in application code
   - Improper integration or error handling by consuming applications
   - Infrastructure security of systems running the library

3. **Denial of Service**
   - Resource exhaustion from excessive ID generation
   - Performance degradation under extreme load

4. **Build System Issues**
   - Build failures unrelated to security
   - CMake or vcpkg configuration problems (unless they introduce vulnerabilities)

## Security Considerations

### Cryptographic Dependencies

This library relies on **OpenSSL 3.x** for cryptographic operations:

- **SHA3-512**: NIST FIPS-202 compliant (domain separation byte 0x06)
- **CSPRNG**: `RAND_bytes()` for cryptographically secure random number generation
- **Validation**: OpenSSL 3.x is FIPS 140-3 validated

**Important**: Keep OpenSSL updated to receive security patches. On non-Windows platforms, use system package managers for timely updates:

```bash
# macOS
brew upgrade openssl@3

# Ubuntu/Debian
sudo apt-get update && sudo apt-get upgrade libssl-dev

# Fedora/RHEL
sudo dnf upgrade openssl-devel
```

### Thread Safety

The library is designed for concurrent use:

- Atomic counter operations prevent race conditions
- Fingerprint singleton is thread-safe (C++11+ guarantees)
- Extensively tested with 10-20 concurrent threads

However, ensure your application properly handles concurrent calls and error conditions.

### Uniqueness Guarantees

Collision resistance depends on:

1. **Sufficient ID length** (minimum 4, recommended 24+ characters)
2. **System entropy** (hostname, process ID, environment variables)
3. **Cryptographic quality randomness** (OpenSSL CSPRNG)
4. **Proper timestamp resolution** (nanosecond precision when available)

**Warning**: IDs shorter than 24 characters have higher collision probability. Use longer IDs (28-32 characters) for high-volume production systems.

### System Fingerprinting

The library collects system information for fingerprinting:

- Hostname (with fallback to random hex string if unavailable)
- Process ID
- Environment variables (sorted alphabetically, then hashed)

**Privacy Note**: While this data is hashed with SHA3-512, be aware that generated IDs incorporate system characteristics. In privacy-sensitive contexts, consider the implications of correlation attacks.

### Endianness and Portability

The library uses **little-endian serialization** via `boost::endian` for cross-platform compatibility. This ensures consistent ID generation across different architectures (x86, ARM, big-endian systems).

## Vulnerability Database Monitoring

We actively monitor:

- [National Vulnerability Database (NVD)](https://nvd.nist.gov/)
- [OpenSSL Security Advisories](https://www.openssl.org/news/vulnerabilities.html)
- [Boost Security](https://www.boost.org/users/news/)
- [GitHub Security Advisories](https://github.com/advisories)
- [Renovate automated dependency updates](https://github.com/renovatebot/renovate)

Dependencies are managed via vcpkg with baseline pinning for reproducible builds. Security updates trigger immediate baseline updates.

## Security Testing

### Current Testing

The project includes comprehensive security-relevant tests:

- **Thread safety tests**: Concurrent generation from 10-20 threads
- **Uniqueness validation**: Up to 50,000 IDs without duplicates
- **Boundary testing**: Length validation, edge cases
- **Platform abstraction tests**: CSPRNG, environment handling

### Recommended External Testing

We encourage security researchers to:

1. **Fuzz test** the library with tools like AFL++, libFuzzer
2. **Static analysis** using tools like Clang Static Analyzer, Coverity
3. **Dynamic analysis** with AddressSanitizer, UndefinedBehaviorSanitizer
4. **Cryptographic validation** against NIST test vectors

Enable sanitizers during development:
```bash
cmake --preset <platform>-<arch>-debug -DENABLE_SANITIZERS=ON
```

## Acknowledgments

We appreciate responsible disclosure and will acknowledge security researchers who help improve libcuid2:

- Security researchers are credited in release notes (unless anonymity is requested)
- Hall of Fame section for significant contributions (coming soon)

## Additional Resources

- **Manual Pages**: `man 7 libcuid2` for security properties and algorithm details
- **API Documentation**: `man 3 libcuid2` for complete library API reference
- **Code of Conduct**: See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md)
- **Contributing**: Please open an issue on GitHub to discuss contributions

## Contact

For all security-related inquiries:

**Email**: security@projects.visus.io

For general questions and non-security issues, please use GitHub Issues.

---

Last Updated: 2026-01-10
