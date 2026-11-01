#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>
#include "workbench/thermal/solver_1d.hpp"

#define TEST_ASSERT(cond, msg) \
    do { if (!(cond)) { throw std::runtime_error(std::string("FAIL: ") + (msg)); } } while(0)

using namespace enginex::thermal;

void test_exact_solution_known_points() {
    std::cout << "Running test_exact_solution_known_points...\n";
    RodParameters p; // L=1, A=1e-4, k=10, q=10000, T_left=100, T_right=20

    // T(0) must equal T_left = 100.0
    double T_0 = Solver1D::exact_solution(0.0, p);
    TEST_ASSERT(std::fabs(T_0 - 100.0) < 1e-12, "T(0) != 100.0");

    // T(L) must equal T_right = 20.0
    double T_L = Solver1D::exact_solution(p.L, p);
    TEST_ASSERT(std::fabs(T_L - 20.0) < 1e-12, "T(L) != 20.0");

    // T(L/2) = 100 + (20-100)*0.5 + (10000 / (2*10)) * 0.5 * (1 - 0.5)
    //        = 100 - 40 + 500 * 0.25 = 60 + 125 = 185.0
    double T_mid = Solver1D::exact_solution(0.5, p);
    TEST_ASSERT(std::fabs(T_mid - 185.0) < 1e-12, "T(L/2) != 185.0");

    std::cout << "  PASS: Exact solution at known points verified.\n";
}

void test_solve_boundary_enforcement() {
    std::cout << "Running test_solve_boundary_enforcement...\n";
    RodParameters p;
    auto res = Solver1D::solve(p, 10);
    TEST_ASSERT(res.is_ok(), "solve(10) failed");

    const auto& sol = res.value();
    TEST_ASSERT(std::fabs(sol.T_fe[0] - p.T_left) < 1e-12, "T_fe[0] does not match Dirichlet left BC");
    TEST_ASSERT(std::fabs(sol.T_fe[sol.N] - p.T_right) < 1e-12, "T_fe[N] does not match Dirichlet right BC");

    std::cout << "  PASS: Dirichlet boundary conditions enforced exactly.\n";
}

void test_solve_n10_accuracy() {
    std::cout << "Running test_solve_n10_accuracy...\n";
    RodParameters p;
    auto res = Solver1D::solve(p, 10);
    TEST_ASSERT(res.is_ok(), "solve(10) failed");

    const auto& sol = res.value();
    // For 1D Poisson with linear elements and uniform heat generation,
    // nodal values are exact! (Green's function property of 1D linear FE)
    for (size_t i = 0; i <= sol.N; ++i) {
        TEST_ASSERT(std::fabs(sol.T_fe[i] - sol.T_exact[i]) < 1e-10, 
            "Nodal values must be exact for 1D Poisson with linear elements");
    }

    // Midpoint error for quadratic exact with linear element of length h:
    // Error at midpoint is |T_exact(x_mid) - T_interp(x_mid)| = (q / (8k)) * h^2
    // For p: q=10000, k=10 -> q/(8k) = 10000/80 = 125.
    // h = 0.1 -> h^2 = 0.01 -> error = 125 * 0.01 = 1.25 °C
    double expected_mid_err = (p.q / (8.0 * p.k)) * (sol.h * sol.h);
    TEST_ASSERT(std::fabs(sol.max_midpoint_error - expected_mid_err) < 1e-10,
        "Midpoint error should match theoretical (q/8k)*h^2");

    std::cout << "  PASS: N=10 accuracy and theoretical midpoint error confirmed: " 
              << sol.max_midpoint_error << " °C.\n";
}

void test_solve_n80_accuracy() {
    std::cout << "Running test_solve_n80_accuracy...\n";
    RodParameters p;
    auto res = Solver1D::solve(p, 80);
    TEST_ASSERT(res.is_ok(), "solve(80) failed");

    const auto& sol = res.value();
    // h = 1/80 = 0.0125. h^2 = 0.00015625. Midpoint error = 125 * 0.00015625 = 0.01953125 °C
    TEST_ASSERT(sol.max_midpoint_error < 0.02, "N=80 max midpoint error must be < 0.02 °C");

    std::cout << "  PASS: N=80 max midpoint error verified: " << sol.max_midpoint_error << " °C.\n";
}

void test_mesh_convergence() {
    std::cout << "Running test_mesh_convergence...\n";
    RodParameters p;
    auto res = Solver1D::mesh_study(p, {10, 20, 40, 80});
    TEST_ASSERT(res.is_ok(), "mesh_study failed");

    const auto& study = res.value();
    TEST_ASSERT(study.solutions.size() == 4, "Expected 4 solutions");

    // Check monotonic decrease of midpoint error and O(h^2) convergence ratio ~ 4.0
    for (size_t i = 1; i < study.solutions.size(); ++i) {
        double prev_err = study.solutions[i - 1].max_midpoint_error;
        double curr_err = study.solutions[i].max_midpoint_error;
        TEST_ASSERT(curr_err < prev_err, "Error must decrease monotonically with mesh refinement");

        double ratio = prev_err / curr_err;
        // Since h halves each step and theoretical error is proportional to h^2, ratio should be 4.0
        TEST_ASSERT(std::fabs(ratio - 4.0) < 1e-6, "Convergence ratio must be exactly 4.0 for O(h^2)");
    }

    std::cout << "  PASS: Monotonic error decrease and exact O(h^2) convergence ratio 4.0 verified.\n";
}

void test_solve_invalid_N() {
    std::cout << "Running test_solve_invalid_N...\n";
    RodParameters p;
    auto res_0 = Solver1D::solve(p, 0);
    TEST_ASSERT(res_0.is_err(), "solve(0) must fail");

    auto res_1 = Solver1D::solve(p, 1);
    TEST_ASSERT(res_1.is_err(), "solve(1) must fail");

    std::cout << "  PASS: Invalid mesh sizes rejected.\n";
}

void test_csv_export_format() {
    std::cout << "Running test_csv_export_format...\n";
    RodParameters p;
    auto res = Solver1D::mesh_study(p, {10, 20});
    TEST_ASSERT(res.is_ok(), "mesh_study failed");

    const std::string& csv = res.value().csv_export;
    std::string expected_header = "N,h,max_midpoint_error_C,node_0_T_C,node_mid_T_C,node_N_T_C\n";
    TEST_ASSERT(csv.rfind(expected_header, 0) == 0, "CSV header does not match expected format");

    std::cout << "  PASS: CSV export format verified.\n";
}

void test_solve_non_finite_parameters() {
    std::cout << "Running test_solve_non_finite_parameters...\n";
    double RodParameters::* fields[] = {
        &RodParameters::L,
        &RodParameters::A,
        &RodParameters::k,
        &RodParameters::q,
        &RodParameters::T_left,
        &RodParameters::T_right
    };
    const double invalid_values[] = {
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()
    };

    for (const auto field : fields) {
        for (const auto invalid_value : invalid_values) {
            RodParameters p;
            p.*field = invalid_value;
            TEST_ASSERT(
                Solver1D::solve(p, 10).is_err(),
                "Every thermal parameter must reject NaN and infinity");
        }
    }

    std::cout << "  PASS: NaN and positive/negative infinity rejected for every thermal parameter.\n";
}

void test_invalid_physical_ranges() {
    std::cout << "Running test_invalid_physical_ranges...\n";
    RodParameters p;

    p.L = 0.0;
    TEST_ASSERT(Solver1D::solve(p, 10).is_err(), "Zero rod length must fail");
    p = RodParameters{};
    p.A = -1.0;
    TEST_ASSERT(Solver1D::solve(p, 10).is_err(), "Negative cross-sectional area must fail");
    p = RodParameters{};
    p.k = 0.0;
    TEST_ASSERT(Solver1D::solve(p, 10).is_err(), "Zero conductivity must fail");
    p = RodParameters{};
    p.T_left = -273.16;
    TEST_ASSERT(Solver1D::solve(p, 10).is_err(), "Temperature below absolute zero must fail");
    p = RodParameters{};
    p.T_right = -273.16;
    TEST_ASSERT(Solver1D::solve(p, 10).is_err(), "Temperature below absolute zero must fail");
    p = RodParameters{};
    p.q = -10000.0;
    TEST_ASSERT(Solver1D::solve(p, 10).is_ok(), "Finite negative heat generation (heat sink) is valid");

    std::cout << "  PASS: Invalid physical ranges rejected; signed heat source remains supported.\n";
}

void test_mesh_element_limit() {
    std::cout << "Running test_mesh_element_limit...\n";
    RodParameters p;

    TEST_ASSERT(kMaxMeshElements == 1000, "Maximum mesh element count must remain documented as 1000");
    auto excessive_solve = Solver1D::solve(p, kMaxMeshElements + 1);
    TEST_ASSERT(excessive_solve.is_err(), "solve above the element limit must fail before allocation");
    TEST_ASSERT(
        excessive_solve.error().message.find("must not exceed 1000") != std::string::npos,
        "Excessive mesh error must state the maximum element count");

    auto excessive_study = Solver1D::mesh_study(p, {10, kMaxMeshElements + 1});
    TEST_ASSERT(excessive_study.is_err(), "mesh study above the element limit must fail");

    std::vector<size_t> too_many_meshes(kMaxMeshStudySizes + 1, 10);
    TEST_ASSERT(
        Solver1D::mesh_study(p, too_many_meshes).is_err(),
        "Mesh studies above the mesh-count limit must fail");

    auto at_limit = Solver1D::solve(p, kMaxMeshElements);
    TEST_ASSERT(at_limit.is_ok(), "The maximum supported mesh must remain valid");
    TEST_ASSERT(
        at_limit.value().T_fe.size() == kMaxMeshElements + 1,
        "Maximum supported mesh must return all nodal temperatures");

    std::cout << "  PASS: Element and study-count limits enforced; N=1000 remains supported.\n";
}

int main() {
    std::cout << "=== ENGINEX 1D Thermal FEA Solver Unit Tests ===\n";
    try {
        test_exact_solution_known_points();
        test_solve_boundary_enforcement();
        test_solve_n10_accuracy();
        test_solve_n80_accuracy();
        test_mesh_convergence();
        test_solve_invalid_N();
        test_csv_export_format();
        test_solve_non_finite_parameters();
        test_invalid_physical_ranges();
        test_mesh_element_limit();
        std::cout << "\nALL 10 THERMAL SOLVER TESTS PASSED!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED: " << e.what() << "\n";
        return 1;
    }
}
