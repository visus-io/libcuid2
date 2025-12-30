#define BOOST_TEST_MODULE FingerprintTest

#include <algorithm>
#include <cstdint>
#include <set>
#include <thread>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "cuid2/fingerprint.hpp"
#include "cuid2/platform.hpp"

BOOST_AUTO_TEST_SUITE(FingerprintTests)

BOOST_AUTO_TEST_CASE(test_fingerprint_non_empty)
{
    const auto& FINGERPRINT = visus::cuid2::Fingerprint::get();

    BOOST_TEST(!FINGERPRINT.empty());
}

BOOST_AUTO_TEST_CASE(test_fingerprint_singleton)
{
    const auto& FP1 = visus::cuid2::Fingerprint::get();
    const auto& FP2 = visus::cuid2::Fingerprint::get();
    const auto& FP3 = visus::cuid2::Fingerprint::get();

    // Verify singleton behavior (same pointer)
    BOOST_TEST(&FP1 == &FP2);
    BOOST_TEST(&FP2 == &FP3);

    // Verify value consistency
    BOOST_TEST(FP1.size() == FP2.size());
    BOOST_TEST(std::equal(FP1.begin(), FP1.end(), FP2.begin(), FP2.end()));
}

BOOST_AUTO_TEST_CASE(test_fingerprint_environment_ordering)
{
    const auto ENV_VARS = visus::cuid2::platform::get_environment_variables();

    // Extract keys from the map and check if they're sorted
    std::vector<std::string> keys;
    keys.reserve(ENV_VARS.size());
    for (const auto& [key, value] : ENV_VARS) {
        keys.push_back(key);
    }
    BOOST_TEST(std::is_sorted(keys.begin(), keys.end()));
}

BOOST_AUTO_TEST_CASE(test_fingerprint_thread_safety)
{
    constexpr int NUM_THREADS = 10;
    constexpr int ITERATIONS = 100;

    std::vector<std::jthread> threads;
    std::vector<const std::vector<uint8_t>*> pointers(static_cast<size_t>(NUM_THREADS) * ITERATIONS);

    threads.reserve(NUM_THREADS);

    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        threads.emplace_back([thread_idx, &pointers]() {
            for (int idx = 0; idx < ITERATIONS; ++idx) {
                const auto& fingerprint = visus::cuid2::Fingerprint::get();
                pointers[(thread_idx * ITERATIONS) + idx] = &fingerprint;
            }
        });
    }

    // Explicitly join to ensure threads complete before accessing results
    for (auto& thread : threads) {
        thread.join();
    }

    const void* FIRST_PTR = pointers[0];
    for (size_t i = 1; i < pointers.size(); ++i) {
        BOOST_TEST(pointers[i] == FIRST_PTR);
    }
}

BOOST_AUTO_TEST_CASE(test_fingerprint_concurrent_access_stability)
{
    constexpr int NUM_THREADS = 20;
    constexpr int ITERATIONS = 500;

    std::atomic ready_threads{0};
    std::atomic start{false};

    std::vector<std::jthread> threads;
    std::vector<std::vector<uint8_t>> thread_copies(NUM_THREADS);

    threads.reserve(NUM_THREADS);

    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        threads.emplace_back([thread_idx, &thread_copies, &ready_threads, &start]() {
            ++ready_threads;

            while (!start.load()) {
                std::this_thread::yield();
            }

            for (int idx = 0; idx < ITERATIONS; ++idx) {
                const auto& fingerprint = visus::cuid2::Fingerprint::get();
                if (idx == 0) {
                    thread_copies[thread_idx] = fingerprint;
                }
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

    const auto& FIRST_COPY = thread_copies[0];
    for (size_t i = 1; i < thread_copies.size(); ++i) {
        BOOST_TEST(thread_copies[i].size() == FIRST_COPY.size());
        BOOST_TEST(std::equal(thread_copies[i].begin(), thread_copies[i].end(), FIRST_COPY.begin(), FIRST_COPY.end()));
    }
}

BOOST_AUTO_TEST_CASE(test_fingerprint_deterministic_structure)
{
    const std::string HOSTNAME = visus::cuid2::platform::get_hostname();
    const int PID = visus::cuid2::platform::get_process_id();
    const auto ENV_VARS = visus::cuid2::platform::get_environment_variables();

    std::vector<uint8_t> expected;

    std::copy(HOSTNAME.begin(), HOSTNAME.end(), std::back_inserter(expected));

    const auto PID_U32 = static_cast<uint32_t>(PID);

    expected.push_back(static_cast<uint8_t>((PID_U32 >> 0) & 0xFF));
    expected.push_back(static_cast<uint8_t>((PID_U32 >> 8) & 0xFF));
    expected.push_back(static_cast<uint8_t>((PID_U32 >> 16) & 0xFF));
    expected.push_back(static_cast<uint8_t>((PID_U32 >> 24) & 0xFF));

    for (const auto& [key, value] : ENV_VARS) {
        std::copy(key.begin(), key.end(), std::back_inserter(expected));
        expected.push_back('=');
        std::copy(value.begin(), value.end(), std::back_inserter(expected));
    }

    const auto& ACTUAL = visus::cuid2::Fingerprint::get();

    BOOST_TEST(ACTUAL.size() == expected.size());
    BOOST_TEST(std::equal(ACTUAL.begin(), ACTUAL.end(), expected.begin(), expected.end()));
}

BOOST_AUTO_TEST_SUITE_END()
