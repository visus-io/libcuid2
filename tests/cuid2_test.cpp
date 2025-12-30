#define BOOST_TEST_MODULE Cuid2Test
#define BOOST_TEST_DYN_LINK

#include <algorithm>
#include <chrono>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "cuid2/cuid2.hpp"

BOOST_AUTO_TEST_SUITE(Cuid2Tests)

namespace {
    /// Helper function to check if a character is a valid base36 character (0-9, a-z)
    bool is_base36_char(const char chr) {
        return (chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'z');
    }

    /// Helper function to check if a character is a lowercase letter (a-z)
    bool is_lowercase_letter(const char chr) {
        return chr >= 'a' && chr <= 'z';
    }

    /// Helper function to validate CUID2 format
    bool is_valid_cuid2_format(const std::string& cuid, const size_t expected_length) {
        if (cuid.length() != expected_length) {
            return false;
        }

        if (cuid.empty()) {
            return false;
        }

        if (!is_lowercase_letter(cuid[0])) {
            return false;
        }

        return std::all_of(cuid.begin(), cuid.end(), is_base36_char);
    }
} // anonymous namespace

BOOST_AUTO_TEST_CASE(test_generate_default_length)
{
    const std::string CUID = visus::cuid2::generate();

    BOOST_TEST(CUID.length() == visus::cuid2::DEFAULT_LENGTH);
    BOOST_TEST(is_valid_cuid2_format(CUID, visus::cuid2::DEFAULT_LENGTH));
}

BOOST_AUTO_TEST_CASE(test_generate_all_valid_lengths)
{
    for (int length = visus::cuid2::MIN_CUID2_LENGTH; length <= visus::cuid2::MAX_CUID2_LENGTH; ++length) {
        const std::string CUID = visus::cuid2::generate(length);

        BOOST_TEST(static_cast<int>(CUID.length()) == length);
        BOOST_TEST(is_valid_cuid2_format(CUID, length));
    }
}

BOOST_AUTO_TEST_CASE(test_exception_length_too_small)
{
    BOOST_CHECK_THROW(visus::cuid2::generate(3), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(2), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(1), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(0), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_exception_length_too_large)
{
    BOOST_CHECK_THROW(visus::cuid2::generate(33), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(50), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(100), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_exception_negative_length)
{
    BOOST_CHECK_THROW(visus::cuid2::generate(-1), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(-10), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(-100), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(test_exception_boundary_values)
{
    // Test exact boundary violations
    BOOST_CHECK_THROW(visus::cuid2::generate(visus::cuid2::MIN_CUID2_LENGTH - 1), std::invalid_argument);
    BOOST_CHECK_THROW(visus::cuid2::generate(visus::cuid2::MAX_CUID2_LENGTH + 1), std::invalid_argument);

    // Test that exact boundaries do NOT throw
    BOOST_CHECK_NO_THROW(visus::cuid2::generate(visus::cuid2::MIN_CUID2_LENGTH));
    BOOST_CHECK_NO_THROW(visus::cuid2::generate(visus::cuid2::MAX_CUID2_LENGTH));
}

BOOST_AUTO_TEST_CASE(test_exception_message_content)
{
    // Verify exception messages contain relevant constraint information
    try {
        visus::cuid2::generate(0);
        BOOST_FAIL("Expected std::invalid_argument to be thrown");
    } catch (const std::invalid_argument& error) {
        const std::string MESSAGE(error.what());
        BOOST_TEST(!MESSAGE.empty());
        BOOST_TEST(MESSAGE.find("4") != std::string::npos);
        BOOST_TEST(MESSAGE.find("32") != std::string::npos);
    }

    try {
        visus::cuid2::generate(100);
        BOOST_FAIL("Expected std::invalid_argument to be thrown");
    } catch (const std::invalid_argument& error) {
        const std::string MESSAGE(error.what());
        BOOST_TEST(!MESSAGE.empty());
        BOOST_TEST(MESSAGE.find("4") != std::string::npos);
        BOOST_TEST(MESSAGE.find("32") != std::string::npos);
    }
}

BOOST_AUTO_TEST_CASE(test_uniqueness_different_lengths)
{
    std::set<std::string> ids_short;
    std::set<std::string> ids_long;

    for (int i = 0; i < 500; ++i) {
        ids_short.insert(visus::cuid2::generate(8));
        ids_long.insert(visus::cuid2::generate(32));
    }

    BOOST_TEST(ids_short.size() == 500U);
    BOOST_TEST(ids_long.size() == 500U);
}

BOOST_AUTO_TEST_CASE(test_thread_safety)
{
    constexpr int NUM_THREADS = 10;
    constexpr int IDS_PER_THREAD = 1000;

    std::vector<std::jthread> threads;
    std::vector<std::vector<std::string>> thread_ids(NUM_THREADS);

    threads.reserve(NUM_THREADS);

    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        threads.emplace_back([thread_idx, &thread_ids]() {
            thread_ids[thread_idx].reserve(IDS_PER_THREAD);
            for (int idx = 0; idx < IDS_PER_THREAD; ++idx) {
                thread_ids[thread_idx].push_back(visus::cuid2::generate());
            }
        });
    }

    // Explicitly join to ensure threads complete before accessing results
    for (auto& thread : threads) {
        thread.join();
    }

    std::set<std::string> all_ids;
    for (const auto& ids : thread_ids) {
        for (const auto& id : ids) {
            all_ids.insert(id);
        }
    }

    BOOST_TEST(all_ids.size() == static_cast<size_t>(NUM_THREADS * IDS_PER_THREAD));
}

BOOST_AUTO_TEST_CASE(test_concurrent_generation_stability)
{
    constexpr int NUM_THREADS = 20;
    constexpr int ITERATIONS = 500;

    std::atomic ready_threads{0};
    std::atomic start{false};

    std::vector<std::jthread> threads;
    std::vector<std::set<std::string>> thread_sets(NUM_THREADS);

    threads.reserve(NUM_THREADS);

    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        threads.emplace_back([thread_idx, &thread_sets, &ready_threads, &start] {
            ++ready_threads;

            while (!start.load()) {
                std::this_thread::yield();
            }

            for (int idx = 0; idx < ITERATIONS; ++idx) {
                thread_sets[thread_idx].insert(visus::cuid2::generate());
            }
        });
    }

    while (ready_threads.load() < NUM_THREADS) {
        std::this_thread::yield();
    }

    start.store(true);

    // Explicitly join to ensure threads complete before accessing results
    for (auto& thread : threads) {
        thread.join();
    }

    std::set<std::string> all_unique_ids;
    size_t total_count = 0;

    for (const auto& thread_set : thread_sets) {
        total_count += thread_set.size();
        for (const auto& id : thread_set) {
            all_unique_ids.insert(id);
        }
    }

    BOOST_TEST(all_unique_ids.size() == total_count);
    BOOST_TEST(all_unique_ids.size() == static_cast<size_t>(NUM_THREADS * ITERATIONS));
}

BOOST_AUTO_TEST_CASE(test_timestamp_ordering)
{
    std::vector<std::string> ids;
    ids.reserve(10);

    for (int i = 0; i < 10; ++i) {
        ids.push_back(visus::cuid2::generate());
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    for (size_t i = 1; i < ids.size(); ++i) {
        BOOST_TEST(ids[i] != ids[i - 1]);
    }
}

BOOST_AUTO_TEST_CASE(test_no_common_prefixes)
{
    std::vector<std::string> ids;
    ids.reserve(100);

    for (int i = 0; i < 100; ++i) {
        ids.push_back(visus::cuid2::generate());
    }

    std::set<std::string> prefixes_3char;
    std::set<std::string> prefixes_5char;

    for (const auto& id : ids) {
        if (id.length() >= 3) {
            prefixes_3char.insert(id.substr(0, 3));
        }
        if (id.length() >= 5) {
            prefixes_5char.insert(id.substr(0, 5));
        }
    }

    BOOST_TEST(prefixes_3char.size() > 70U);
    BOOST_TEST(prefixes_5char.size() > 90U);
}

BOOST_AUTO_TEST_CASE(test_rapid_generation_no_duplicates)
{
    std::vector<std::string> ids;
    ids.reserve(50000);

    for (int i = 0; i < 50000; ++i) {
        ids.push_back(visus::cuid2::generate());
    }

    std::set<std::string> unique_ids(ids.begin(), ids.end());

    BOOST_TEST(unique_ids.size() == ids.size());
}

BOOST_AUTO_TEST_CASE(test_multiple_lengths_concurrent)
{
    constexpr int NUM_THREADS = 5;
    const std::vector LENGTHS = {8, 16, 20, 24, 32};

    std::vector<std::jthread> threads;
    std::vector<std::vector<std::string>> thread_ids(NUM_THREADS);

    threads.reserve(NUM_THREADS);

    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        threads.emplace_back([thread_idx, &thread_ids, &LENGTHS]() {
            const int LENGTH = LENGTHS[thread_idx];
            thread_ids[thread_idx].reserve(300);
            for (int idx = 0; idx < 300; ++idx) {
                thread_ids[thread_idx].push_back(visus::cuid2::generate(LENGTH));
            }
        });
    }

    // Explicitly join to ensure threads complete before accessing results
    for (auto& thread : threads) {
        thread.join();
    }

    std::set<std::string> all_ids;
    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        BOOST_TEST(thread_ids[thread_idx].size() == 300U);
        for (const auto& id : thread_ids[thread_idx]) {
            BOOST_TEST(static_cast<int>(id.length()) == LENGTHS[thread_idx]);
            BOOST_TEST(is_valid_cuid2_format(id, LENGTHS[thread_idx]));
            all_ids.insert(id);
        }
    }

    BOOST_TEST(all_ids.size() == static_cast<size_t>(NUM_THREADS * 300));
}

BOOST_AUTO_TEST_CASE(test_short_ids_have_higher_collision_rate)
{
    std::set<std::string> ids;

    for (int i = 0; i < 200; ++i) {
        ids.insert(visus::cuid2::generate(4));
    }

    BOOST_TEST(ids.size() >= 150U);
}

BOOST_AUTO_TEST_SUITE_END()
