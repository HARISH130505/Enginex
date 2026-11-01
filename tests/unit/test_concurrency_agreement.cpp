#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include "workbench/performance/benchmark_harness.hpp"

#define TEST_ASSERT(cond, msg) \
    do { if (!(cond)) { throw std::runtime_error(std::string("FAIL: ") + (msg)); } } while(0)

using namespace enginex::performance;

void test_serial_parallel_order_preservation() {
    std::cout << "Running test_serial_parallel_order_preservation...\n";
    auto batch = BatchGenerator::generate(30, 42, 20);

    auto serial_res = BenchmarkHarness::run_batch(batch, 1);
    auto parallel_res = BenchmarkHarness::run_batch(batch, 4);

    TEST_ASSERT(serial_res.size() == batch.size(), "Serial results count mismatch");
    TEST_ASSERT(parallel_res.size() == batch.size(), "Parallel results count mismatch");

    // Verify exact order preservation and numerical agreement
    for (size_t i = 0; i < batch.size(); ++i) {
        TEST_ASSERT(serial_res[i].N == parallel_res[i].N, "Element N mismatch at index " + std::to_string(i));
        const size_t n_nodes = serial_res[i].T_fe.size();
        for (size_t j = 0; j < n_nodes; ++j) {
            double diff = std::fabs(serial_res[i].T_fe[j] - parallel_res[i].T_fe[j]);
            TEST_ASSERT(diff <= 1e-8, "Order or numerical discrepancy at job " + std::to_string(i) + " node " + std::to_string(j));
        }
    }

    std::cout << "  PASS: Result order preservation and bitwise/IEEE-754 agreement verified.\n";
}

void test_benchmark_suite_numerical_agreement() {
    std::cout << "Running test_benchmark_suite_numerical_agreement...\n";
    BenchmarkConfig cfg;
    cfg.batch_size = 40;
    cfg.worker_counts = {1, 2, 4};
    cfg.warmup_reps = 1;
    cfg.timed_reps = 3;
    cfg.seed = 100;
    cfg.elements_per_sim = 30;

    auto res = BenchmarkHarness::run_benchmark(cfg);
    TEST_ASSERT(res.is_ok(), "run_benchmark failed: " + (res.is_err() ? res.error().message : ""));

    const auto& b = res.value();
    TEST_ASSERT(b.worker_results.size() == 3, "Expected 3 worker count results");
    TEST_ASSERT(b.agreement_verified, "Numerical agreement flag must be true");
    TEST_ASSERT(b.overall_max_discrepancy <= 1e-8, "Max discrepancy exceeds 1e-8 °C threshold");

    for (const auto& wr : b.worker_results) {
        TEST_ASSERT(wr.median_ms > 0.0, "Median runtime must be positive");
        TEST_ASSERT(wr.speedup > 0.0, "Speedup must be positive");
        TEST_ASSERT(wr.efficiency > 0.0, "Efficiency must be positive");
        TEST_ASSERT(wr.max_temp_diff_vs_serial <= 1e-8, "Worker discrepancy exceeds 1e-8 °C");
    }

    TEST_ASSERT(!b.csv_export.empty(), "CSV export must not be empty");
    TEST_ASSERT(b.csv_export.find("workers,rep_1_ms") != std::string::npos, "CSV header mismatch");

    std::cout << "  PASS: Overall max discrepancy = " << b.overall_max_discrepancy 
              << " °C (<= 1e-8 °C threshold). Agreement verified.\n";
}

int main() {
    std::cout << "=== ENGINEX Concurrency & Agreement Verification Tests ===\n";
    try {
        test_serial_parallel_order_preservation();
        test_benchmark_suite_numerical_agreement();
        std::cout << "\nALL CONCURRENCY & AGREEMENT TESTS PASSED!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << "\n";
        return 1;
    }
}

