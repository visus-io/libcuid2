# libcuid2 Manual Pages

This directory contains manual pages (manpages) for the libcuid2 library and tools.

## Available Manpages

### Section 1: User Commands
- **cuid2gen.1** - Manual page for the `cuid2gen` command-line tool
  - Usage: `man 1 cuid2gen` or `man cuid2gen`
  - Describes the CLI tool for generating CUID2 identifiers

### Section 3: Library Functions
- **libcuid2.3** - API reference for the libcuid2 library
  - Usage: `man 3 libcuid2`
  - Comprehensive reference for the C++ API
  - Includes function signatures, parameters, return values, and examples
  - Covers error handling and thread safety

### Section 7: Miscellaneous
- **libcuid2.7** - Overview and architecture documentation
  - Usage: `man 7 libcuid2`
  - In-depth explanation of the CUID2 algorithm
  - Architecture and design decisions
  - Cryptographic properties and security considerations
  - Performance characteristics
  - Cross-platform compatibility details

## Viewing Manpages

### Before Installation
To view manpages before installation, use the full path:

```bash
# View the CLI tool manpage
man ./man/man1/cuid2gen.1

# View the API reference
man ./man/man3/libcuid2.3

# View the overview/architecture
man ./man/man7/libcuid2.7
```

### After Installation
After running `make install` or `cmake --install`, the manpages will be installed to the system manpage directories (typically `/usr/local/share/man` or `/usr/share/man`). You can then view them with:

```bash
# View the CLI tool manpage
man cuid2gen
# or explicitly:
man 1 cuid2gen

# View the API reference
man 3 libcuid2

# View the overview
man 7 libcuid2
```

## Installation

The manpages are automatically installed when you install the library:

```bash
# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Install (may require sudo)
sudo cmake --install build
```

This will install:
- `cuid2gen.1` to `${CMAKE_INSTALL_MANDIR}/man1/`
- `libcuid2.3` to `${CMAKE_INSTALL_MANDIR}/man3/`
- `libcuid2.7` to `${CMAKE_INSTALL_MANDIR}/man7/`

On most systems, `CMAKE_INSTALL_MANDIR` defaults to `/usr/local/share/man` or `/usr/share/man`.

## Manpage Sections

The manpage section numbers follow standard Unix conventions:

- **Section 1**: User commands (executable programs or shell commands)
- **Section 3**: Library functions (C/C++ library APIs)
- **Section 7**: Miscellaneous (conventions, overviews, protocols, etc.)

## Format

All manpages are written in the traditional `troff` format with `man` macros. They can be:
- Viewed with the `man` command
- Converted to other formats with `groff`, `mandoc`, or `pandoc`
- Rendered in web browsers via `man2html`

## Quick Reference

| What do you need? | Which manpage? | Command |
|------------------|----------------|---------|
| How to use the CLI tool | cuid2gen.1 | `man cuid2gen` |
| How to call the C++ API | libcuid2.3 | `man 3 libcuid2` |
| How CUID2 works internally | libcuid2.7 | `man 7 libcuid2` |
| Code examples | libcuid2.3 | `man 3 libcuid2` |
| Algorithm details | libcuid2.7 | `man 7 libcuid2` |
| Thread safety info | libcuid2.3 or libcuid2.7 | `man 3 libcuid2` |
| Platform support | libcuid2.7 | `man 7 libcuid2` |

## Generating Other Formats

### PDF
```bash
man -t ./man/man3/libcuid2.3 | ps2pdf - libcuid2.pdf
```

### HTML
```bash
groff -mandoc -Thtml ./man/man3/libcuid2.3 > libcuid2.html
```

### Plain Text
```bash
man ./man/man3/libcuid2.3 | col -b > libcuid2.txt
```

## Localization

libcuid2 manpages support multiple languages through a standard localization structure.

### Available Languages

| Language | Code | Status |
|----------|------|--------|
| English (default) | (root) | ✅ Complete |
| German (Deutsch) | `de` | 📝 Ready for translation |
| Spanish (Español) | `es` | 📝 Ready for translation |
| French (Français) | `fr` | 📝 Ready for translation |
| Italian (Italiano) | `it` | 📝 Ready for translation |
| Japanese (日本語) | `ja` | 📝 Ready for translation |
| Korean (한국어) | `ko` | 📝 Ready for translation |
| Polish (Polski) | `pl` | 📝 Ready for translation |
| Portuguese European (Português) | `pt` | 📝 Ready for translation |
| Portuguese Brazilian (Português) | `pt_BR` | 📝 Ready for translation |
| Russian (Русский) | `ru` | 📝 Ready for translation |
| Chinese Simplified (简体中文) | `zh_CN` | 📝 Ready for translation |

### Viewing Translated Manpages

The `man` command automatically selects the appropriate language based on your system locale:

```bash
# Use system default locale
man cuid2gen

# Explicitly set language
LANG=de_DE.UTF-8 man cuid2gen        # German
LANG=es_ES.UTF-8 man 7 libcuid2      # Spanish
LANG=fr_FR.UTF-8 man 3 libcuid2      # French
LANG=it_IT.UTF-8 man cuid2gen        # Italian
LANG=ja_JP.UTF-8 man 3 libcuid2      # Japanese
LANG=ko_KR.UTF-8 man 7 libcuid2      # Korean
LANG=pl_PL.UTF-8 man cuid2gen        # Polish
LANG=pt_PT.UTF-8 man 3 libcuid2      # Portuguese (European)
LANG=pt_BR.UTF-8 man 7 libcuid2      # Portuguese (Brazilian)
LANG=ru_RU.UTF-8 man cuid2gen        # Russian
LANG=zh_CN.UTF-8 man 3 libcuid2      # Simplified Chinese

# View directly (before installation)
man -l de/man1/cuid2gen.1
man -l pt/man3/libcuid2.3
man -l pt_BR/man7/libcuid2.7
man -l ko/man1/cuid2gen.1
```

### Directory Structure

```
man/
├── man1/              # English (default)
├── man3/
├── man7/
├── de/                # German
│   ├── man1/
│   ├── man3/
│   └── man7/
├── fr/                # French
│   └── ...
└── LOCALIZATION.md    # Translation guidelines
```

### Contributing Translations

See [LOCALIZATION.md](LOCALIZATION.md) for comprehensive guidelines on:
- Adding new languages
- Translation workflow
- Testing translations
- Maintaining consistency

We welcome translations! Help make libcuid2 accessible to developers worldwide. 🌍

## Contributing

When updating manpages:
1. Follow the existing style and structure
2. Use standard troff/man macros (.SH, .TP, .PP, .IP, etc.)
3. Keep lines under 80 characters where possible
4. Test formatting with `man` before committing
5. Update the date in the `.TH` line
6. Maintain consistency across all manpages
7. For translations, see [LOCALIZATION.md](LOCALIZATION.md)

## See Also

- CUID2 specification: https://github.com/paralleldrive/cuid2
- Linux man-pages project: https://www.kernel.org/doc/man-pages/
- man(7) - format of manual pages
- groff(1) - document formatting system
