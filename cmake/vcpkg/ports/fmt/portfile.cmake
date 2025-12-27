# System library stub port for fmt
# This port tells vcpkg to skip building fmt and rely on the system installation
# Note: Unlike other overlay ports, this one has a fallback mechanism in case fmt is not available

if(VCPKG_TARGET_IS_WINDOWS)
    message(FATAL_ERROR
        "This overlay port is for non-Windows platforms only.\n"
        "Windows builds should use the official vcpkg fmt port.\n"
        "Update CMakePresets.json to use 'default-windows' preset.")
endif()

# On non-Windows platforms, assume system fmt is available
# CMake's find_package(fmt CONFIG) will validate this during the main build
message(STATUS "Using system-installed fmt")
message(STATUS "If fmt is not installed, the main CMake configure will fail.")
message(STATUS "Install fmt via your package manager (e.g., 'brew install fmt', 'apt-get install libfmt-dev')")

# Mark this as an empty package - vcpkg will not install anything
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
