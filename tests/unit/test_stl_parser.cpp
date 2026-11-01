#include "workbench/geometry/stl_parser.hpp"
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

void test_stl_valid_cube() {
    std::cout << "[RUN] Testing ASCII STL parser with valid 12-triangle cube...\n";
    std::ifstream file = open_test_file("data/geometry/sample_cube.stl");
    TEST_ASSERT(file.is_open(), "Could not locate or open sample_cube.stl in candidate search paths");

    auto result = StlParser::parse_ascii(file, "sample_cube.stl");
    TEST_ASSERT(result.is_ok(), "Expected valid STL parse but received error");
    
    const auto& mesh = result.value();
    TEST_ASSERT(mesh.name == "cube", "Expected solid name 'cube'");
    TEST_ASSERT(mesh.triangles.size() == 12, "Expected exactly 12 triangles for cube");

    // Verify bounding box
    TEST_ASSERT(std::abs(mesh.bbox.min.x - 0.0) < 1e-6, "Bounding box min.x must be 0.0");
    TEST_ASSERT(std::abs(mesh.bbox.min.y - 0.0) < 1e-6, "Bounding box min.y must be 0.0");
    TEST_ASSERT(std::abs(mesh.bbox.min.z - 0.0) < 1e-6, "Bounding box min.z must be 0.0");

    TEST_ASSERT(std::abs(mesh.bbox.max.x - 1.0) < 1e-6, "Bounding box max.x must be 1.0");
    TEST_ASSERT(std::abs(mesh.bbox.max.y - 1.0) < 1e-6, "Bounding box max.y must be 1.0");
    TEST_ASSERT(std::abs(mesh.bbox.max.z - 1.0) < 1e-6, "Bounding box max.z must be 1.0");

    auto dims = mesh.bbox.dimensions();
    TEST_ASSERT(std::abs(dims.x - 1.0) < 1e-6, "Dimensions dx must be 1.0");
    TEST_ASSERT(std::abs(dims.y - 1.0) < 1e-6, "Dimensions dy must be 1.0");
    TEST_ASSERT(std::abs(dims.z - 1.0) < 1e-6, "Dimensions dz must be 1.0");

    std::cout << "[PASS] Valid cube parsed successfully: 12 triangles, bbox [0,0,0] to [1,1,1].\n";
}

void test_stl_empty_file() {
    std::cout << "[RUN] Testing STL negative case: empty file...\n";
    auto result = StlParser::parse_ascii_string("", "empty.stl");
    TEST_ASSERT(result.is_err(), "Empty file must fail parsing");
    TEST_ASSERT(result.error().code == enginex::core::StatusCode::InvalidArgument, "Must return InvalidArgument");
    std::cout << "[PASS] Empty STL correctly rejected with: " << result.error().message << "\n";
}

void test_stl_not_ascii() {
    std::cout << "[RUN] Testing STL negative case: missing solid keyword...\n";
    auto result = StlParser::parse_ascii_string("binary header 12345678", "bad.stl");
    TEST_ASSERT(result.is_err(), "Missing solid header must fail");
    TEST_ASSERT(result.error().code == enginex::core::StatusCode::InvalidArgument, "Must return InvalidArgument");
    std::cout << "[PASS] Non-ASCII STL correctly rejected with: " << result.error().message << "\n";
}

void test_stl_truncated_facet() {
    std::cout << "[RUN] Testing STL negative case: truncated facet (2 vertices instead of 3)...\n";
    std::string content = 
        "solid test\n"
        "  facet normal 0 0 1\n"
        "    outer loop\n"
        "      vertex 0 0 0\n"
        "      vertex 1 0 0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid test\n";
    auto result = StlParser::parse_ascii_string(content, "truncated.stl");
    TEST_ASSERT(result.is_err(), "Truncated facet must fail");
    TEST_ASSERT(result.error().code == enginex::core::StatusCode::InvalidArgument, "Must return InvalidArgument");
    std::cout << "[PASS] Truncated facet correctly rejected with: " << result.error().message << "\n";
}

void test_stl_nan_vertex() {
    std::cout << "[RUN] Testing STL negative case: NaN vertex coordinate...\n";
    std::string content = 
        "solid test\n"
        "  facet normal 0 0 1\n"
        "    outer loop\n"
        "      vertex 0 0 0\n"
        "      vertex NaN 1 0\n"
        "      vertex 0 1 0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid test\n";
    auto result = StlParser::parse_ascii_string(content, "nan.stl");
    TEST_ASSERT(result.is_err(), "NaN vertex coordinate must fail");
    TEST_ASSERT(result.error().code == enginex::core::StatusCode::InvalidArgument, "Must return InvalidArgument");
    std::cout << "[PASS] NaN vertex coordinate correctly rejected with: " << result.error().message << "\n";
}

void test_stl_triangle_limit() {
    std::cout << "[RUN] Testing STL negative case: exceeding max triangles limit...\n";
    std::ifstream file = open_test_file("data/geometry/sample_cube.stl");
    TEST_ASSERT(file.is_open(), "Could not open sample_cube.stl");

    // Configured max triangles = 5 (sample has 12)
    auto result = StlParser::parse_ascii(file, "cube.stl", 50*1024*1024, 5);
    TEST_ASSERT(result.is_err(), "Exceeding triangle limit must fail");
    TEST_ASSERT(result.error().code == enginex::core::StatusCode::InvalidArgument, "Must return InvalidArgument");
    std::cout << "[PASS] Triangle limit correctly enforced with: " << result.error().message << "\n";
}

void test_stl_file_size_limit() {
    std::cout << "[RUN] Testing STL negative case: exceeding max file size limit...\n";
    std::string content = "solid test\n";
    std::istringstream iss(content);
    auto result = StlParser::parse_ascii(iss, "limit.stl", 5);
    TEST_ASSERT(result.is_err(), "Exceeding file size limit must fail");
    std::cout << "[PASS] File size limit correctly enforced with: " << result.error().message << "\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "  ENGINEX ASCII STL Parser Test Suite\n";
    std::cout << "==================================================\n";

    try {
        test_stl_valid_cube();
        test_stl_empty_file();
        test_stl_not_ascii();
        test_stl_truncated_facet();
        test_stl_nan_vertex();
        test_stl_triangle_limit();
        test_stl_file_size_limit();
        std::cout << "\n>>> ALL ASCII STL PARSER TESTS PASSED! <<<\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] Unexpected exception: " << e.what() << "\n";
        return 1;
    }
}
