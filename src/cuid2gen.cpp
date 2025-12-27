#include <charconv>
#include <string_view>
#include <system_error>

#include "cuid2/cuid2.hpp"

#include <fmt/core.h>

namespace {
    void print_help(std::string_view program_name) noexcept {
        fmt::print(
            "Usage: {} [OPTIONS]\n\n"
            "Generate a collision-resistant CUID2 identifier.\n\n"
            "Options:\n"
            "  -l, --length <num>   Length of the generated ID (default: 24, min: 4, max: 32)\n"
            "  -h, --help           Display this help message and exit\n\n"
            "Examples:\n"
            "  {}                 # Generate default length (24) CUID2\n"
            "  {} -l 16           # Generate 16-character CUID2\n"
            "  {} --length 32     # Generate maximum length (32) CUID2\n",
            program_name, program_name, program_name, program_name);
    }

    [[nodiscard]] constexpr bool is_help_flag(std::string_view arg) noexcept {
        return arg == "-h" || arg == "--help";
    }

    [[nodiscard]] constexpr bool is_length_flag(std::string_view arg) noexcept {
        return arg == "-l" || arg == "--length";
    }
} // anonymous namespace

int main(const int argc, char* argv[]) {
    using namespace visus::cuid2;

    int length = DEFAULT_LENGTH;

    int i = 1;
    while (i < argc) {
        const std::string_view ARG{argv[i]};

        if (is_help_flag(ARG)) {
            print_help(argv[0]);
            return 0;
        }

        if (is_length_flag(ARG)) {
            if (i + 1 >= argc) {
                fmt::print(stderr, "Error: {} requires an argument\n\n", ARG);
                print_help(argv[0]);
                return 1;
            }

            ++i;
            const std::string_view LENGTH_ARG{argv[i]};
            const auto [ptr, ec] = std::from_chars(
                LENGTH_ARG.data(),
                LENGTH_ARG.data() + LENGTH_ARG.size(),
                length);

            if (ec != std::errc{} || ptr != LENGTH_ARG.data() + LENGTH_ARG.size()) {
                fmt::print(stderr, "Error: Invalid length value '{}'\n\n", LENGTH_ARG);
                print_help(argv[0]);
                return 1;
            }

            ++i;
            continue;
        }

        fmt::print(stderr, "Error: Unknown option '{}'\n\n", ARG);
        print_help(argv[0]);
        return 1;
    }

    try {
        fmt::print("{}\n", generate(length));
        return 0;
    } catch (const std::exception& e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }
}
