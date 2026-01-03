/// @file cuid2_export.hpp
/// @brief Cross-platform DLL export/import macros for Windows shared library builds
///
/// This header provides the CUID2_API macro for proper symbol visibility on Windows
/// when building or using the library as a shared library (DLL). On non-Windows
/// platforms, the macro expands to nothing as no special decoration is needed.
///
/// CMake automatically defines cuid2_EXPORTS when building the cuid2 shared library,
/// which triggers __declspec(dllexport) to export symbols. When other code uses the
/// library, __declspec(dllimport) is used to import the symbols.
///
/// Note: Test executables also define cuid2_EXPORTS since they compile library sources
/// directly for testing internal components. This ensures consistent symbol visibility.

#ifndef LIBCUID2_EXPORT_HPP
#define LIBCUID2_EXPORT_HPP

#ifdef _WIN32
  #ifdef cuid2_EXPORTS
    /// @brief Symbol export/import decoration for Windows DLL builds
    /// Expands to __declspec(dllexport) when building the library,
    /// __declspec(dllimport) when using the library, and nothing on non-Windows.
    #define CUID2_API __declspec(dllexport)
  #else
    #define CUID2_API __declspec(dllimport)
  #endif
#else
  #define CUID2_API
#endif

#endif // LIBCUID2_EXPORT_HPP
