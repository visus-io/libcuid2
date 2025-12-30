# ==============================================================================
# Project Configuration
# ==============================================================================
cmake_minimum_required(VERSION 4.0)
project(libcuid2 VERSION 0.0.1 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Compiler Flags
# ==============================================================================
# Warning flags (all configurations)
if(MSVC)
    add_compile_options(
        /W4           # High warning level
        /WX-          # Don't treat warnings as errors (developers can override)
        /permissive-  # Standards conformance mode
    )
else()
    add_compile_options(
        -Wall         # Enable most warnings
        -Wextra       # Additional warnings
        -Wpedantic    # Strict ISO C++ compliance
        -Wshadow      # Warn on variable shadowing
        -Wnon-virtual-dtor  # Warn on non-virtual destructors
        -Wcast-align  # Warn on pointer casts with alignment issues
        -Wunused      # Warn on unused variables/functions
        -Woverloaded-virtual  # Warn on overloaded virtual functions
        -pipe         # Use pipes instead of temp files (faster compilation)
    )
endif()

# ==============================================================================
# Security Hardening Flags
# ==============================================================================
if(MSVC)
    add_compile_options(/GS)      # Buffer security checks (stack protector)
    add_compile_options(/sdl)     # Enable additional security checks
else()
    # GCC/Clang (including MinGW)
    add_compile_options(-fstack-protector-strong)  # Stack buffer overflow protection

    # _FORTIFY_SOURCE requires glibc (not available on Windows/MinGW)
    if(NOT WIN32)
        add_compile_options(-D_FORTIFY_SOURCE=2)   # Compile-time and runtime checks
    endif()

    # Control flow integrity (x86_64 only, GCC 8+ / Clang 8+)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag(-fcf-protection=full HAS_CF_PROTECTION)
        if(HAS_CF_PROTECTION)
            add_compile_options(-fcf-protection=full)  # Control flow integrity
        endif()
    endif()

    # NOTE: The following security flags are intentionally disabled:
    # - -fstack-clash-protection: Interferes with C++ exception handling on some platforms
    # - -mbranch-protection=standard (ARM64): Conflicts with exception unwinding on macOS ARM64
    # These flags caused exceptions to not be caught properly during testing.
endif()

# ==============================================================================
# Build Configuration Flags
# ==============================================================================
if(MSVC)
    # Debug configuration
    add_compile_options($<$<CONFIG:Debug>:/Od>)      # No optimization
    add_compile_options($<$<CONFIG:Debug>:/Zi>)      # Debug symbols
    add_compile_options($<$<CONFIG:Debug>:/RTC1>)    # Runtime checks

    # Release configuration
    add_compile_options($<$<CONFIG:Release>:/O2>)    # Optimize for speed
    add_compile_options($<$<CONFIG:Release>:/GL>)    # Whole program optimization
    add_compile_options($<$<CONFIG:Release>:/DNDEBUG>)  # Disable assertions
    add_link_options($<$<CONFIG:Release>:/LTCG>)     # Link-time code generation
    add_link_options($<$<CONFIG:Release>:/OPT:REF>)  # Remove unreferenced functions
    add_link_options($<$<CONFIG:Release>:/OPT:ICF>)  # Identical COMDAT folding
else()
    # Debug configuration
    add_compile_options($<$<CONFIG:Debug>:-O0>)      # No optimization
    add_compile_options($<$<CONFIG:Debug>:-g3>)      # Maximum debug symbols
    add_compile_options($<$<CONFIG:Debug>:-fno-omit-frame-pointer>)  # Preserve frame pointers

    # Release configuration
    add_compile_options($<$<CONFIG:Release>:-O2>)    # Standard optimization
    add_compile_options($<$<CONFIG:Release>:-DNDEBUG>)  # Disable assertions
    add_compile_options($<$<CONFIG:Release>:-flto>)  # Link-time optimization
    add_compile_options($<$<CONFIG:Release>:-ffunction-sections>)  # Separate functions
    add_compile_options($<$<CONFIG:Release>:-fdata-sections>)      # Separate data
    add_link_options($<$<CONFIG:Release>:-flto>)

    if(APPLE)
        add_link_options($<$<CONFIG:Release>:-Wl,-dead_strip>)     # Strip unused code (macOS)
    else()
        add_link_options($<$<CONFIG:Release>:-Wl,--gc-sections>)   # Strip unused code (Linux/BSD)
        add_link_options($<$<CONFIG:Release>:-Wl,--strip-all>)     # Strip all symbols
    endif()
endif()

# Optional: Enable sanitizers for Debug builds (can be disabled with -DENABLE_SANITIZERS=OFF)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers in Debug builds" OFF)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    add_compile_options(-fsanitize=address -fsanitize=undefined)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

# Optional: Enable code coverage for Debug builds (can be enabled with -DENABLE_COVERAGE=ON)
option(ENABLE_COVERAGE "Enable code coverage instrumentation in Debug builds" OFF)
if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    add_compile_options(--coverage -fprofile-update=atomic)
    add_link_options(--coverage)
endif()

# ==============================================================================
# Dependencies
# ==============================================================================
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(fmt CONFIG REQUIRED)

# ==============================================================================
# Platform-Specific Configuration
# ==============================================================================
if(WIN32)
    set(PLATFORM_LIBS kernel32)
else()
    set(PLATFORM_LIBS)
endif()

# ==============================================================================
# Build Type Configuration
# ==============================================================================
# Windows always builds shared libraries
if(WIN32)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries (forced ON for Windows)" FORCE)
    message(STATUS "Windows detected: building shared library (forced)")
else()
    option(BUILD_SHARED_LIBS "Build shared libraries" ON)
    if(BUILD_SHARED_LIBS)
        message(STATUS "Building shared library")
    else()
        message(STATUS "Building static library")
    endif()
endif()

# ==============================================================================
# Library Target
# ==============================================================================
add_library(cuid2
    src/cuid2.cpp
    src/fingerprint.cpp
    src/counter.cpp
    src/platform.cpp
    src/utils.cpp
)

add_library(cuid2::cuid2 ALIAS cuid2)

set_target_properties(cuid2 PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

target_include_directories(cuid2
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cuid2
    PRIVATE
        ${PLATFORM_LIBS}
        OpenSSL::Crypto
        Boost::boost
        fmt::fmt
)

# ==============================================================================
# Testing
# ==============================================================================
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    enable_testing()

    # Counter test
    add_executable(counter_test
        tests/counter_test.cpp
        src/counter.cpp
        src/platform.cpp
    )

    target_include_directories(counter_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(counter_test
        PRIVATE
            ${PLATFORM_LIBS}
            OpenSSL::Crypto
            Boost::unit_test_framework
            fmt::fmt
    )

    add_test(NAME CounterTest COMMAND counter_test)

    # Fingerprint test
    add_executable(fingerprint_test
        tests/fingerprint_test.cpp
        src/fingerprint.cpp
        src/platform.cpp
    )

    target_include_directories(fingerprint_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(fingerprint_test
        PRIVATE
            ${PLATFORM_LIBS}
            OpenSSL::Crypto
            Boost::unit_test_framework
            fmt::fmt
    )

    add_test(NAME FingerprintTest COMMAND fingerprint_test)

    # Cuid2 test
    add_executable(cuid2_test
        tests/cuid2_test.cpp
        src/cuid2.cpp
        src/counter.cpp
        src/fingerprint.cpp
        src/platform.cpp
        src/utils.cpp
    )

    target_include_directories(cuid2_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(cuid2_test
        PRIVATE
            ${PLATFORM_LIBS}
            OpenSSL::Crypto
            Boost::unit_test_framework
            fmt::fmt
    )

    add_test(NAME Cuid2Test COMMAND cuid2_test)

    # Utils test
    add_executable(utils_test
        tests/utils_test.cpp
        src/utils.cpp
        src/platform.cpp
    )

    target_include_directories(utils_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(utils_test
        PRIVATE
            ${PLATFORM_LIBS}
            OpenSSL::Crypto
            Boost::unit_test_framework
            fmt::fmt
    )

    add_test(NAME UtilsTest COMMAND utils_test)

    # Platform test
    add_executable(platform_test
        tests/platform_test.cpp
        src/platform.cpp
    )

    target_include_directories(platform_test
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(platform_test
        PRIVATE
            ${PLATFORM_LIBS}
            OpenSSL::Crypto
            Boost::unit_test_framework
            fmt::fmt
    )

    add_test(NAME PlatformTest COMMAND platform_test)
endif()

# ==============================================================================
# CLI Executable
# ==============================================================================
add_executable(cuid2gen
    src/cuid2gen.cpp
)

target_link_libraries(cuid2gen
    PRIVATE
        cuid2::cuid2
        fmt::fmt
)

# ==============================================================================
# Installation
# ==============================================================================
include(GNUInstallDirs)

# Install library
install(TARGETS cuid2
    EXPORT cuid2Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

# Install CLI executable
install(TARGETS cuid2gen
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install manpages (English - default locale)
install(FILES man/man1/cuid2gen.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)

install(FILES man/man3/libcuid2.3
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man3
)

install(FILES man/man7/libcuid2.7
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
)

# Install localized manpages (if available)
# Each language directory is installed to ${CMAKE_INSTALL_MANDIR}/<lang>/
foreach(LANG de es fr it ja ko pl pt pt_BR ru zh_CN)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/man/${LANG})
        install(DIRECTORY man/${LANG}/
            DESTINATION ${CMAKE_INSTALL_MANDIR}/${LANG}
            FILES_MATCHING
            PATTERN "*.1"
            PATTERN "*.3"
            PATTERN "*.7"
            PATTERN "README.md" EXCLUDE
        )
    endif()
endforeach()

# Install CMake package configuration
install(EXPORT cuid2Targets
    FILE cuid2Targets.cmake
    NAMESPACE cuid2::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cuid2
)

# Create package config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/cuid2ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cuid2Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cuid2Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cuid2
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cuid2Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cuid2ConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cuid2
)
