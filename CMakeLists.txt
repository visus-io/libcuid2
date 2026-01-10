# ==============================================================================
# Project Configuration
# ==============================================================================
cmake_minimum_required(VERSION 3.22)
project(libcuid2 VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# Compiler Flags
# ==============================================================================
if(MSVC)
    add_compile_options(/W4)
    add_compile_options(/WX-)
    add_compile_options(/permissive-)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Wcast-align
        -Wunused
        -Woverloaded-virtual
        -pipe
    )
endif()

# Security hardening
if(MSVC)
    add_compile_options(/GS)
    add_compile_options(/sdl)
else()
    add_compile_options(-fstack-protector-strong)

    if(NOT WIN32)
        add_compile_options(-D_FORTIFY_SOURCE=2)
    endif()

    # Control flow integrity (x86_64 only)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag(-fcf-protection=full HAS_CF_PROTECTION)
        if(HAS_CF_PROTECTION)
            add_compile_options(-fcf-protection=full)
        endif()
    endif()

    # NOTE: -fstack-clash-protection and -mbranch-protection interfere with
    # C++ exception handling and are intentionally disabled
endif()

# Build configurations
if(MSVC)
    add_compile_options(
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Debug>:/Zi>
        $<$<CONFIG:Debug>:/RTC1>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/DNDEBUG>
    )
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:Release>:/OPT:REF>
        $<$<CONFIG:Release>:/OPT:ICF>
    )
else()
    add_compile_options(
        $<$<CONFIG:Debug>:-O0>
        $<$<CONFIG:Debug>:-g3>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Release>:-DNDEBUG>
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-ffunction-sections>
        $<$<CONFIG:Release>:-fdata-sections>
    )
    add_link_options($<$<CONFIG:Release>:-flto>)

    if(APPLE)
        add_link_options($<$<CONFIG:Release>:-Wl,-dead_strip>)
    else()
        add_link_options(
            $<$<CONFIG:Release>:-Wl,--gc-sections>
            $<$<CONFIG:Release>:-Wl,--strip-all>
        )
    endif()
endif()

# Optional features
option(ENABLE_SANITIZERS "Enable sanitizers in Debug builds" OFF)
if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    add_compile_options(-fsanitize=address)
    add_compile_options(-fsanitize=undefined)
    add_link_options(-fsanitize=address)
    add_link_options(-fsanitize=undefined)
endif()

option(ENABLE_COVERAGE "Enable code coverage in Debug builds" OFF)
if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    add_compile_options(--coverage)
    add_compile_options(-fprofile-update=atomic)
    add_link_options(--coverage)
endif()

# ==============================================================================
# Dependencies
# ==============================================================================
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
find_package(fmt CONFIG REQUIRED)

if(WIN32)
    set(PLATFORM_LIBS kernel32)
else()
    set(PLATFORM_LIBS)
endif()

# ==============================================================================
# Library Target
# ==============================================================================
add_library(cuid2 SHARED
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

    # Helper function for test targets
    function(add_unit_test TEST_NAME)
        add_executable(${TEST_NAME} ${ARGN})

        target_include_directories(${TEST_NAME}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/include
                ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        target_link_libraries(${TEST_NAME}
            PRIVATE
                ${PLATFORM_LIBS}
                OpenSSL::Crypto
                Boost::unit_test_framework
                fmt::fmt
        )

        # Define cuid2_EXPORTS for test builds to ensure consistent symbol visibility
        # Tests compile library sources directly, so they need the same export macro behavior
        target_compile_definitions(${TEST_NAME} PRIVATE cuid2_EXPORTS)

        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endfunction()

    add_unit_test(counter_test
        tests/counter_test.cpp
        src/counter.cpp
        src/platform.cpp
    )

    add_unit_test(fingerprint_test
        tests/fingerprint_test.cpp
        src/fingerprint.cpp
        src/platform.cpp
    )

    add_unit_test(cuid2_test
        tests/cuid2_test.cpp
        src/cuid2.cpp
        src/counter.cpp
        src/fingerprint.cpp
        src/platform.cpp
        src/utils.cpp
    )

    add_unit_test(utils_test
        tests/utils_test.cpp
        src/utils.cpp
        src/platform.cpp
    )

    add_unit_test(platform_test
        tests/platform_test.cpp
        src/platform.cpp
    )
endif()

# ==============================================================================
# CLI Executable
# ==============================================================================
add_executable(cuid2gen src/cuid2gen.cpp)

target_link_libraries(cuid2gen
    PRIVATE
        cuid2::cuid2
        fmt::fmt
)

# ==============================================================================
# Installation
# ==============================================================================
include(GNUInstallDirs)

install(TARGETS cuid2
    EXPORT cuid2Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS cuid2gen
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install Windows runtime dependencies (DLLs)
if(WIN32)
    install(IMPORTED_RUNTIME_ARTIFACTS cuid2 cuid2gen
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Install manpages
install(FILES
    man/man1/cuid2gen.1
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
)

install(FILES
    man/man3/libcuid2.3
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man3
)

install(FILES
    man/man7/libcuid2.7
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man7
)

# Install localized manpages
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

# CMake package configuration
install(EXPORT cuid2Targets
    FILE cuid2Targets.cmake
    NAMESPACE cuid2::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cuid2
)

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
