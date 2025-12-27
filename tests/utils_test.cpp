#define BOOST_TEST_MODULE UtilsTest

#include <array>
#include <vector>

#include <boost/test/unit_test.hpp>

#include "cuid2/utils.hpp"
#include "cuid2/platform.hpp"

BOOST_AUTO_TEST_SUITE(UtilsTests)

BOOST_AUTO_TEST_CASE(test_encode_base36_empty_input)
{
    const std::vector<uint8_t> EMPTY_DATA{};

    const std::string RESULT = visus::cuid2::utils::encode_base36(EMPTY_DATA);

    BOOST_TEST(RESULT == "0");
}

BOOST_AUTO_TEST_CASE(test_encode_base36_all_zeros)
{
    const std::vector<uint8_t> ZERO_DATA{0, 0, 0, 0};

    const std::string RESULT = visus::cuid2::utils::encode_base36(ZERO_DATA);

    BOOST_TEST(RESULT == "0");
}

BOOST_AUTO_TEST_CASE(test_encode_base36_single_byte)
{
    const std::vector<uint8_t> DATA{42};

    const std::string RESULT = visus::cuid2::utils::encode_base36(DATA);

    BOOST_TEST(RESULT == "16");
}

BOOST_AUTO_TEST_CASE(test_encode_base36_known_values)
{
    const std::vector<uint8_t> DATA_255{255};
    const std::string RESULT_255 = visus::cuid2::utils::encode_base36(DATA_255);
    BOOST_TEST(RESULT_255 == "73");

    const std::vector<uint8_t> DATA_256{1, 0};
    const std::string RESULT_256 = visus::cuid2::utils::encode_base36(DATA_256);
    BOOST_TEST(RESULT_256 == "74");
}

BOOST_AUTO_TEST_CASE(test_generate_prefix_valid_letters)
{
    for (int i = 0; i < 100; ++i) {
        const char PREFIX = visus::cuid2::utils::generate_prefix();

        BOOST_TEST(PREFIX >= 'a');
        BOOST_TEST(PREFIX <= 'z');
    }
}

BOOST_AUTO_TEST_CASE(test_get_timestamp_ticks_positive)
{
    const int64_t TIMESTAMP = visus::cuid2::utils::get_timestamp_ticks();

    BOOST_TEST(TIMESTAMP > 0);
}

BOOST_AUTO_TEST_CASE(test_get_timestamp_ticks_monotonic)
{
    const int64_t T1 = visus::cuid2::utils::get_timestamp_ticks();
    const int64_t T2 = visus::cuid2::utils::get_timestamp_ticks();

    BOOST_TEST(T2 >= T1);
}

BOOST_AUTO_TEST_SUITE_END()
