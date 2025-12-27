# System library stub port for Boost.Test
# This port tells vcpkg to skip building and rely on the system Boost installation

if(VCPKG_TARGET_IS_WINDOWS)
    message(FATAL_ERROR
        "This overlay port is for non-Windows platforms only.\n"
        "Windows builds should use the official vcpkg boost-test port.\n"
        "Update CMakePresets.json to use 'default-windows' preset.")
endif()

# On non-Windows platforms, assume system Boost.Test is available
# CMake's FindBoost will validate this during the main build
message(STATUS "Using system-installed Boost for Boost.Test (unit_test_framework)")
message(STATUS "If Boost is not installed, the main CMake configure will fail with instructions.")

# Mark this as an empty package - vcpkg will not install anything
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
