#include "workbench/geometry/csv_parser.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <vector>

using namespace enginex::geometry;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "[ASSERTION FAILED] " << msg << " (" << #cond << ") at " << __FILE__ << ":" << __LINE__ << "\n"; \
            throw std::runtime_error(std::string("Assertion failed: ") + (msg)); \
        } \
    } while(0)

std::ifstream open_test_file(const std::string& rel_path) {
    std::vector<std::string> prefixes = {"", "../", "../../", "../../../"};
    for (const auto& p : prefixes) {
        std::ifstream f(p + rel_path);
        if (f.is_open()) return f;
    }
    return std::ifstream();
}

void test_csv_valid_synthetic() {
    std::cout << "[RUN] Testing ANSYS nodal CSV parser with valid synthetic dataset...\n";
    std::ifstream file = open_test_file("data/temperature/synthetic_ansys_sample.csv");
    TEST_ASSERT(file.is_open(), "Could not locate or open synthetic_ansys_sample.csv in search paths");

    auto result = CsvParser::parse_ansys_csv(file, "synthetic_ansys_sample.csv");
    TEST_ASSERT(result.is_ok(), "Expected valid CSV parse but received error");

    const auto& ds = result.value();
    TEST_ASSERT(ds.nodes.size() == 40, "Expected 40 nodes in synthetic dataset");
    TEST_ASSERT(std::abs(ds.temp_min - 20.0) < 1e-4, "Min temperature must be 20.0 C");
    TEST_ASSERT(std::abs(ds.temp_max - 100.0) < 1e-4, "Max temperature must be 100.0 C");
    TEST_ASSERT(ds.temp_mean > 20.0 && ds.temp_mean < 100.0, "Mean temperature must lie within [20, 100]");

    // Verify first node
    TEST_ASSERT(ds.nodes[0].node_id == 1, "First node ID must be 1");
    TEST_ASSERT(std::abs(ds.nodes[0].temperature_C - 100.0) < 1e-4, "First node temperature must be 100.0 C");

    // Verify bounding box
    TEST_ASSERT(std::abs(ds.bbox.min.x - 0.0) < 1e-4, "Bounding box min.x must be 0.0");
    TEST_ASSERT(std::abs(ds.bbox.max.x - 1.0) < 1e-4, "Bounding box max.x must be 1.0");

    std::cout << "[PASS] Synthetic CSV parsed: 40 nodes, T in [" << ds.temp_min << ", " << ds.temp_max << "] °C, mean: " << ds.temp_mean << " °C.\n";
}

void test_csv_column_reordering() {
    std::cout << "[RUN] Testing CSV column reordering tolerance...\n";
    std::ifstream file = open_test_file("tests/fixtures/reordered_valid.csv");
    TEST_ASSERT(file.is_open(), "Could not locate reordered_valid.csv");

    auto result = CsvParser::parse_ansys_csv(file, "reordered_valid.csv");
    TEST_ASSERT(result.is_ok(), "Reordered CSV columns must parse cleanly");

    const auto& ds = result.value();
    TEST_ASSERT(ds.nodes.size() == 2, "Expected 2 nodes");
    TEST_ASSERT(ds.nodes[0].node_id == 101, "Node ID should be 101");
    TEST_ASSERT(std::abs(ds.nodes[0].temperature_C - 45.5) < 1e-4, "Temperature must be 45.5 C");
    TEST_ASSERT(std::abs(ds.nodes[0].x_m - 0.1) < 1e-4, "x_m must be 0.1");
    TEST_ASSERT(std::abs(ds.nodes[0].y_m - 0.2) < 1e-4, "y_m must be 0.2");
    TEST_ASSERT(std::abs(ds.nodes[0].z_m - 0.5) < 1e-4, "z_m must be 0.5");

    std::cout << "[PASS] Reordered columns correctly identified and parsed by header name.\n";
}

void test_csv_empty_file() {
    std::cout << "[RUN] Testing CSV negative case: empty file...\n";
    auto result = CsvParser::parse_ansys_csv_string("", "empty.csv");
    TEST_ASSERT(result.is_err(), "Empty file must fail parsing");
    std::cout << "[PASS] Empty CSV correctly rejected: " << result.error().message << "\n";
}

void test_csv_missing_header() {
    std::cout << "[RUN] Testing CSV negative case: missing required column...\n";
    std::string content = "node_id,x_m,y_m,z_m\n1,0.0,0.0,0.0\n";
    auto result = CsvParser::parse_ansys_csv_string(content, "missing.csv");
    TEST_ASSERT(result.is_err(), "Missing header must fail parsing");
    std::cout << "[PASS] Missing header column correctly rejected: " << result.error().message << "\n";
}

void test_csv_malformed_number() {
    std::cout << "[RUN] Testing CSV negative case: malformed numeric string...\n";
    std::string content = "node_id,x_m,y_m,z_m,temperature_C\n1,0.0,bad_coord,0.0,100.0\n";
    auto result = CsvParser::parse_ansys_csv_string(content, "bad_num.csv");
    TEST_ASSERT(result.is_err(), "Malformed number must fail parsing");
    std::cout << "[PASS] Malformed number correctly rejected: " << result.error().message << "\n";
}

void test_csv_nan_temperature() {
    std::cout << "[RUN] Testing CSV negative case: NaN temperature value...\n";
    std::string content = "node_id,x_m,y_m,z_m,temperature_C\n1,0.0,0.0,0.0,NaN\n";
    auto result = CsvParser::parse_ansys_csv_string(content, "nan_temp.csv");
    TEST_ASSERT(result.is_err(), "NaN value must fail parsing");
    std::cout << "[PASS] NaN value correctly rejected: " << result.error().message << "\n";
}

void test_csv_duplicate_node_ids() {
    std::cout << "[RUN] Testing CSV duplicate node ID policy (Reject vs KeepFirst)...\n";
    std::string content = 
        "node_id,x_m,y_m,z_m,temperature_C\n"
        "1,0.0,0.0,0.0,100.0\n"
        "2,0.5,0.0,0.0,50.0\n"
        "1,0.2,0.0,0.0,80.0\n";

    // Policy 1: RejectOnDuplicate
    auto result_reject = CsvParser::parse_ansys_csv_string(content, "dup.csv", DuplicatePolicy::RejectOnDuplicate);
    TEST_ASSERT(result_reject.is_err(), "RejectOnDuplicate must fail on duplicate node IDs");
    std::cout << "[PASS] RejectOnDuplicate correctly triggered error: " << result_reject.error().message << "\n";

    // Policy 2: KeepFirst
    auto result_keep = CsvParser::parse_ansys_csv_string(content, "dup.csv", DuplicatePolicy::KeepFirst);
    TEST_ASSERT(result_keep.is_ok(), "KeepFirst must succeed");
    TEST_ASSERT(result_keep.value().nodes.size() == 2, "Expected 2 nodes kept");
    TEST_ASSERT(result_keep.value().duplicate_node_count == 1, "Expected 1 duplicate counted");
    TEST_ASSERT(std::abs(result_keep.value().nodes[0].temperature_C - 100.0) < 1e-4, "Kept node should be first instance (100 C)");
    // Policy 3: Overwrite with recalculated statistics
    auto result_overwrite = CsvParser::parse_ansys_csv_string(content, "dup.csv", DuplicatePolicy::Overwrite);
    TEST_ASSERT(result_overwrite.is_ok(), "Overwrite policy must succeed");
    const auto& ds_over = result_overwrite.value();
    TEST_ASSERT(ds_over.nodes.size() == 2, "Expected 2 nodes after overwrite");
    TEST_ASSERT(ds_over.duplicate_node_count == 1, "Expected 1 duplicate count");
    TEST_ASSERT(std::abs(ds_over.nodes[0].temperature_C - 80.0) < 1e-4, "Overwritten node should have new temperature (80 C)");
    // Recomputed statistics check: nodes are {1: 80.0 C} and {2: 50.0 C}
    // Min should be 50.0, Max should be 80.0, Mean should be 65.0
    TEST_ASSERT(std::abs(ds_over.temp_min - 50.0) < 1e-4, "Recalculated temp_min must be 50.0 C");
    TEST_ASSERT(std::abs(ds_over.temp_max - 80.0) < 1e-4, "Recalculated temp_max must be 80.0 C");
    TEST_ASSERT(std::abs(ds_over.temp_mean - 65.0) < 1e-4, "Recalculated temp_mean must be 65.0 C");
    std::cout << "[PASS] Overwrite policy correctly updated node and recomputed statistics.\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "  ENGINEX ANSYS Nodal CSV Parser Test Suite\n";
    std::cout << "==================================================\n";

    try {
        test_csv_valid_synthetic();
        test_csv_column_reordering();
        test_csv_empty_file();
        test_csv_missing_header();
        test_csv_malformed_number();
        test_csv_nan_temperature();
        test_csv_duplicate_node_ids();
        std::cout << "\n>>> ALL ANSYS CSV PARSER TESTS PASSED! <<<\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] Unexpected exception: " << e.what() << "\n";
        return 1;
    }
}
