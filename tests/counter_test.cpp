#define BOOST_TEST_MODULE CounterTest

#include <atomic>
#include <set>
#include <thread>
#include <vector>

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include "cuid2/counter.hpp"

BOOST_AUTO_TEST_SUITE(CounterTests)

BOOST_AUTO_TEST_CASE(test_counter_increments)
{
    const int64_t VAL1 = visus::cuid2::Counter::next();
    const int64_t VAL2 = visus::cuid2::Counter::next();
    const int64_t VAL3 = visus::cuid2::Counter::next();
    const int64_t VAL4 = visus::cuid2::Counter::next();
    const int64_t VAL5 = visus::cuid2::Counter::next();

    BOOST_TEST(VAL2 == VAL1 + 1);
    BOOST_TEST(VAL3 == VAL2 + 1);
    BOOST_TEST(VAL4 == VAL3 + 1);
    BOOST_TEST(VAL5 == VAL4 + 1);
}

BOOST_AUTO_TEST_CASE(test_counter_uniqueness)
{
    std::set<int64_t> values;
    for (int i = 0; i < 1000; ++i) {
        int64_t const VALUE = visus::cuid2::Counter::next();
        values.insert(VALUE);
    }

    BOOST_TEST(values.size() == 1000U);
}

BOOST_AUTO_TEST_CASE(test_counter_thread_safety)
{
    constexpr int NUM_THREADS = 10;
    constexpr int VALUES_PER_THREAD = 1000;

    std::vector<std::thread> threads;
    std::vector<std::vector<int64_t>> thread_values(NUM_THREADS);

    threads.reserve(NUM_THREADS);

    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        threads.emplace_back([thread_idx, &thread_values]() {
            thread_values[thread_idx].reserve(VALUES_PER_THREAD);
            for (int idx = 0; idx < VALUES_PER_THREAD; ++idx) {
                thread_values[thread_idx].push_back(visus::cuid2::Counter::next());
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::set<int64_t> all_values;
    for (const auto& values : thread_values) {
        for (int64_t const VALUE : values) {
            all_values.insert(VALUE);
        }
    }

    BOOST_TEST(all_values.size() == static_cast<size_t>(NUM_THREADS * VALUES_PER_THREAD));
}

BOOST_AUTO_TEST_CASE(test_counter_ordering_within_thread)
{
    std::vector<int64_t> values;
    values.reserve(100);

    for (int i = 0; i < 100; ++i) {
        values.push_back(visus::cuid2::Counter::next());
    }

    for (size_t i = 1; i < values.size(); ++i) {
        BOOST_TEST(values[i] > values[i-1]);
        BOOST_TEST(values[i] == values[i-1] + 1);
    }
}

BOOST_AUTO_TEST_CASE(test_counter_consistent_increments)
{
    int64_t last_value = visus::cuid2::Counter::next();

    for (int i = 0; i < 10000; ++i) {
        const int64_t CURRENT = visus::cuid2::Counter::next();

        BOOST_TEST(CURRENT == last_value + 1);

        last_value = CURRENT;
    }
}

BOOST_AUTO_TEST_CASE(test_counter_concurrent_access_stability)
{
    constexpr int NUM_THREADS = 20;
    constexpr int ITERATIONS = 500;

    std::atomic ready_threads{0};
    std::atomic start{false};

    std::vector<std::thread> threads;
    std::vector<std::set<int64_t>> thread_sets(NUM_THREADS);

    threads.reserve(NUM_THREADS);

    for (int thread_idx = 0; thread_idx < NUM_THREADS; ++thread_idx) {
        threads.emplace_back([thread_idx, &thread_sets, &ready_threads, &start] {
            ++ready_threads;

            while (!start.load()) {
                std::this_thread::yield();
            }

            for (int idx = 0; idx < ITERATIONS; ++idx) {
                thread_sets[thread_idx].insert(visus::cuid2::Counter::next());
            }
        });
    }

    while (ready_threads.load() < NUM_THREADS) {
        std::this_thread::yield();
    }

    start.store(true);

    for (auto& thread : threads) {
        thread.join();
    }

    std::set<int64_t> all_unique_values;
    size_t total_count = 0;

    for (const auto& thread_set : thread_sets) {
        total_count += thread_set.size();
        for (int64_t const VALUE : thread_set) {
            all_unique_values.insert(VALUE);
        }
    }

    BOOST_TEST(all_unique_values.size() == total_count);
    BOOST_TEST(all_unique_values.size() == static_cast<size_t>(NUM_THREADS * ITERATIONS));
}

BOOST_AUTO_TEST_SUITE_END()
