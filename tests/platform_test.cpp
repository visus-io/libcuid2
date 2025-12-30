#define BOOST_TEST_MODULE PlatformTest

#include <algorithm>
#include <array>
#include <set>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "cuid2/platform.hpp"

BOOST_AUTO_TEST_SUITE(PlatformTests)

BOOST_AUTO_TEST_CASE(test_get_random_int64_produces_varied_values)
{
    std::set<int64_t> values;

    for (int i = 0; i < 100; ++i) {
        values.insert(visus::cuid2::platform::get_random_int64());
    }

    BOOST_TEST(values.size() > 95U);
}

BOOST_AUTO_TEST_CASE(test_get_random_bytes_fills_buffer)
{
    std::array<unsigned char, 32> buffer{};

    visus::cuid2::platform::get_random_bytes(buffer.data(), buffer.size());

    bool all_zeros = true;
    for (const auto BYTE : buffer) {
        if (BYTE != 0) {
            all_zeros = false;
            break;
        }
    }

    BOOST_TEST(!all_zeros);
}

BOOST_AUTO_TEST_CASE(test_get_process_id_consistent)
{
    const int PID1 = visus::cuid2::platform::get_process_id();
    const int PID2 = visus::cuid2::platform::get_process_id();

    BOOST_TEST(PID1 == PID2);
    BOOST_TEST(PID1 > 0);
}

BOOST_AUTO_TEST_CASE(test_get_hostname_not_empty)
{
    const std::string HOSTNAME = visus::cuid2::platform::get_hostname();

    BOOST_TEST(!HOSTNAME.empty());
    BOOST_TEST(HOSTNAME.length() > 0U);
}

BOOST_AUTO_TEST_CASE(test_get_environment_variables_not_empty)
{
    const auto ENV_VARS = visus::cuid2::platform::get_environment_variables();

    BOOST_TEST(!ENV_VARS.empty());
}

BOOST_AUTO_TEST_CASE(test_get_environment_variables_sorted)
{
    const auto ENV_VARS = visus::cuid2::platform::get_environment_variables();

    std::vector<std::string> keys;
    keys.reserve(ENV_VARS.size());

    for (const auto& [key, value] : ENV_VARS) {
        keys.push_back(key);
    }

    const bool IS_SORTED = std::is_sorted(keys.begin(), keys.end(), std::less<std::string>());
    BOOST_TEST(IS_SORTED);
}

BOOST_AUTO_TEST_SUITE_END()
