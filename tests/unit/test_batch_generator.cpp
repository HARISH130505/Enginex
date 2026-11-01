#include <iostream>
#include <cmath>
#include <stdexcept>
#include <string>
#include "workbench/performance/batch_generator.hpp"

#define TEST_ASSERT(cond, msg) \
    do { if (!(cond)) { throw std::runtime_error(std::string("FAIL: ") + (msg)); } } while(0)

using namespace enginex::performance;

void test_batch_generation_count_and_bounds() {
    std::cout << "Running test_batch_generation_count_and_bounds...\n";
    size_t count = 100;
    auto batch = BatchGenerator::generate(count, 42, 40, 1.0, 100.0, 1000.0, 50000.0);

    TEST_ASSERT(batch.size() == count, "Batch size mismatch");

    for (size_t i = 0; i < count; ++i) {
        TEST_ASSERT(batch[i].id == i, "Batch index ID mismatch");
        TEST_ASSERT(batch[i].N == 40, "Element count mismatch");
        TEST_ASSERT(batch[i].params.k >= 1.0 && batch[i].params.k <= 100.0, "k out of bounds [1, 100]");
        TEST_ASSERT(batch[i].params.q >= 1000.0 && batch[i].params.q <= 50000.0, "q out of bounds [1000, 50000]");
        TEST_ASSERT(batch[i].params.L == 1.0, "Length L must be 1.0");
        TEST_ASSERT(batch[i].params.T_left == 100.0, "T_left must be 100.0");
        TEST_ASSERT(batch[i].params.T_right == 20.0, "T_right must be 20.0");
    }

    std::cout << "  PASS: Batch count and parameter bounds verified.\n";
}

void test_deterministic_reproducibility() {
    std::cout << "Running test_deterministic_reproducibility...\n";
    auto b1 = BatchGenerator::generate(50, 1337);
    auto b2 = BatchGenerator::generate(50, 1337);

    TEST_ASSERT(b1.size() == b2.size(), "Batch size mismatch");

    for (size_t i = 0; i < b1.size(); ++i) {
        TEST_ASSERT(b1[i].params.k == b2[i].params.k, "Deterministic k mismatch for same seed");
        TEST_ASSERT(b1[i].params.q == b2[i].params.q, "Deterministic q mismatch for same seed");
    }

    // Different seed should differ
    auto b3 = BatchGenerator::generate(50, 9999);
    bool any_difference = false;
    for (size_t i = 0; i < b1.size(); ++i) {
        if (b1[i].params.k != b3[i].params.k || b1[i].params.q != b3[i].params.q) {
            any_difference = true;
            break;
        }
    }
    TEST_ASSERT(any_difference, "Different seeds must produce different parameter sets");

    std::cout << "  PASS: Deterministic reproducibility across identical seeds confirmed.\n";
}

int main() {
    std::cout << "=== ENGINEX Batch Generator Unit Tests ===\n";
    try {
        test_batch_generation_count_and_bounds();
        test_deterministic_reproducibility();
        std::cout << "\nALL BATCH GENERATOR TESTS PASSED!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << "\n";
        return 1;
    }
}

