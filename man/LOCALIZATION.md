# Manpage Localization Guide

This document describes the localization structure for libcuid2 manual pages and how to contribute translations.

## Directory Structure

The manpage directory follows the standard GNU/Linux localization pattern:

```
man/
├── man1/               # English (default locale)
│   └── cuid2gen.1
├── man3/
│   └── libcuid2.3
├── man7/
│   └── libcuid2.7
├── de/                 # German (Deutsch)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── es/                 # Spanish (Español)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── fr/                 # French (Français)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── it/                 # Italian (Italiano)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── ja/                 # Japanese (日本語)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── ko/                 # Korean (한국어)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── pl/                 # Polish (Polski)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── pt/                 # Portuguese European (Português)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── pt_BR/              # Portuguese Brazilian (Português)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── ru/                 # Russian (Русский)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── zh_CN/              # Chinese Simplified (简体中文)
│   ├── man1/
│   ├── man3/
│   └── man7/
├── LOCALIZATION.md
└── README.md
```

## Supported Languages

Currently prepared for the following languages (ISO 639-1 / ISO 3166-1 codes):

| Code    | Language                         | Status                   |
|---------|----------------------------------|--------------------------|
| (root)  | English                          | ✅ Complete               |
| `de`    | German (Deutsch)                 | 📝 Ready for translation |
| `es`    | Spanish (Español)                | 📝 Ready for translation |
| `fr`    | French (Français)                | 📝 Ready for translation |
| `it`    | Italian (Italiano)               | 📝 Ready for translation |
| `ja`    | Japanese (日本語)                   | 📝 Ready for translation |
| `ko`    | Korean (한국어)                     | 📝 Ready for translation |
| `pl`    | Polish (Polski)                  | 📝 Ready for translation |
| `pt`    | Portuguese European (Português)  | 📝 Ready for translation |
| `pt_BR` | Portuguese Brazilian (Português) | 📝 Ready for translation |
| `ru`    | Russian (Русский)                | 📝 Ready for translation |
| `zh_CN` | Chinese Simplified (简体中文)        | 📝 Ready for translation |

## Adding a New Language

To add support for a new language:

### 1. Create the directory structure

```bash
cd man/
mkdir -p <LANG_CODE>/man{1,3,7}
```

Where `<LANG_CODE>` is the appropriate language code:
- Use ISO 639-1 two-letter codes (e.g., `de`, `fr`, `es`)
- For regional variants, use ISO 3166-1 (e.g., `pt_BR`, `zh_CN`, `zh_TW`)

### 2. Create placeholder README

```bash
cat > <LANG_CODE>/README.md << 'EOF'
# libcuid2 Manual Pages - [Language Name]

This directory contains [Language Name] translations of the libcuid2 manual pages.

## Translation Status

- [ ] cuid2gen.1 - CLI tool manual
- [ ] libcuid2.3 - API reference manual
- [ ] libcuid2.7 - Overview and architecture manual

## Contributing

See ../LOCALIZATION.md for translation guidelines.
EOF
```

### 3. Update CMakeLists.txt

Add the new language to the installation section in `CMakeLists.txt`:

```cmake
# Install localized manpages for <LANG_CODE>
install(DIRECTORY man/<LANG_CODE>/
    DESTINATION ${CMAKE_INSTALL_MANDIR}/<LANG_CODE>
    FILES_MATCHING
    PATTERN "*.1"
    PATTERN "*.3"
    PATTERN "*.7"
)
```

## Translation Guidelines

### General Principles

1. **Preserve Technical Accuracy**: Technical terms, function names, and code examples should remain in English or use standard technical translations for your language
2. **Maintain Structure**: Keep the same section structure as the English version
3. **Update Dates**: Use the translation date in the `.TH` line
4. **Character Encoding**: Use UTF-8 encoding for all files
5. **Consistency**: Use consistent terminology throughout all manpages

### Manpage-Specific Guidelines

#### cuid2gen.1 (CLI Tool)
- Translate user-facing text, descriptions, and examples
- Keep command-line flags (`-l`, `--length`, etc.) untranslated
- Translate option descriptions
- Adapt examples to be culturally appropriate while maintaining technical correctness

#### libcuid2.3 (API Reference)
- Keep C++ code examples and function signatures unchanged
- Translate descriptions, explanations, and comments
- Keep `#include` statements, namespaces, and identifiers in English
- Translate error messages and exception descriptions
- Keep URLs and references as-is (English)

#### libcuid2.7 (Overview)
- Translate all descriptive text
- Keep cryptographic terms consistent with your language's standard terminology
- Translate section headings
- Keep algorithm names and technical acronyms (SHA3-512, CSPRNG, etc.) untranslated or use standard translations

### Format Guidelines

1. **Troff Syntax**: Maintain the same troff/groff markup as the English version
2. **Line Length**: Try to keep lines under 80 characters (may vary for languages with longer words)
3. **Macros**: Do not translate macro names (`.TH`, `.SH`, `.TP`, etc.)
4. **Formatting**: Preserve bold (`.B`), italic (`.I`), and other text formatting

### Example Translation Template

```troff
.TH CUID2GEN 1 "YYYY-MM-DD" "libcuid2" "[Translated: User Commands]"
.SH [TRANSLATED: NAME]
cuid2gen \- [translated description]
.SH [TRANSLATED: SYNOPSIS]
.B cuid2gen
[\fB\-l\fR|\fB\-\-length\fR \fInum\fR]
```

## Testing Translations

After creating a translation, test it with:

```bash
# View the translated manpage
man -l <LANG_CODE>/man1/cuid2gen.1
man -l <LANG_CODE>/man3/libcuid2.3
man -l <LANG_CODE>/man7/libcuid2.7

# Test with language-specific locale
LANG=<LANG_CODE>.UTF-8 man -l <LANG_CODE>/man1/cuid2gen.1
```

Verify:
- ✅ Proper rendering of special characters
- ✅ Correct line wrapping
- ✅ No broken formatting
- ✅ All sections present
- ✅ Technical accuracy maintained

## Installation Behavior

When installed, manpages will be placed in:

```
${CMAKE_INSTALL_MANDIR}/
├── man1/cuid2gen.1                    # English (default)
├── man3/libcuid2.3
├── man7/libcuid2.7
├── de/man1/cuid2gen.1                 # German
├── de/man3/libcuid2.3
├── de/man7/libcuid2.7
└── [other languages...]
```

The `man` command automatically selects the appropriate language based on the `LANG`, `LC_ALL`, or `LC_MESSAGES` environment variables.

## Language Selection at Runtime

Users can view manpages in their preferred language:

```bash
# Use system locale (automatic)
man cuid2gen

# Explicitly specify language
LANG=de_DE.UTF-8 man cuid2gen        # German
LANG=fr_FR.UTF-8 man 3 libcuid2      # French
LANG=ja_JP.UTF-8 man 7 libcuid2      # Japanese

# View specific language version directly
man -l man/de/man1/cuid2gen.1
```

## Translation Workflow

1. **Fork/Clone**: Get the latest version of libcuid2
2. **Choose Language**: Select which language to translate
3. **Copy English Source**: Start with the English manpage as a template
4. **Translate**: Translate following the guidelines above
5. **Test**: View with `man -l` to verify formatting
6. **Update README**: Mark translation as complete in `<LANG_CODE>/README.md`
7. **Submit**: Create a pull request with your translation

## Maintaining Translations

When the English manpages are updated:

1. Check the changelog or git diff for changes
2. Update translated versions to match
3. Update the date in the `.TH` line
4. Test the updated manpage
5. Submit updates

## Getting Help

- Check existing translations for examples
- Review the English source for structure
- See `man 7 groff` for formatting details
- See `man 7 man` for manpage conventions
- Ask in project issues or discussions

## Credits

Translators will be credited in:
- The manpage itself (AUTHOR or TRANSLATORS section)
- The project's CONTRIBUTORS file
- Release notes

Thank you for helping make libcuid2 accessible to developers worldwide! 🌍
