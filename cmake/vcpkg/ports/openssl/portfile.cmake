# System library stub port for OpenSSL
# This port tells vcpkg to skip building OpenSSL and rely on the system installation

if(VCPKG_TARGET_IS_WINDOWS)
    message(FATAL_ERROR
        "This overlay port is for non-Windows platforms only.\n"
        "Windows builds should use the official vcpkg OpenSSL port.\n"
        "Update CMakePresets.json to use 'default-windows' preset.")
endif()

# On non-Windows platforms, assume system OpenSSL is available
# CMake's FindOpenSSL will validate this during the main build
message(STATUS "Using system-installed OpenSSL")
message(STATUS "If OpenSSL is not installed, the main CMake configure will fail with instructions.")

# Mark this as an empty package - vcpkg will not install anything
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
