#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include "workbench/core/result.hpp"
#include "workbench/core/json.hpp"

namespace enginex::thermal {

// Physical parameters for the 1D steady-state heat conduction problem
struct RodParameters {
    double L     = 1.0;       // Rod length (m)
    double A     = 1.0e-4;    // Cross-sectional area (m^2)
    double k     = 10.0;      // Thermal conductivity W/(m·K)
    double q     = 10000.0;   // Volumetric heat generation (W/m^3)
    double T_left  = 100.0;   // Dirichlet BC at x=0 (°C)
    double T_right = 20.0;    // Dirichlet BC at x=L (°C)

    nlohmann::json to_json() const;
};

// Result of solving for a single mesh density N
struct FEASolution {
    size_t N;                         // Number of elements
    double h;                         // Element length L/N
    std::vector<double> x_nodes;      // Nodal coordinates (N+1 values)
    std::vector<double> T_fe;         // FE nodal temperatures (N+1 values)
    std::vector<double> T_exact;      // Exact analytical at nodes (N+1 values)
    std::vector<double> x_midpoints;  // Element midpoint coordinates (N values)
    std::vector<double> T_fe_mid;     // FE interpolated at midpoints (N values)
    std::vector<double> T_exact_mid;  // Exact analytical at midpoints (N values)
    std::vector<double> error_mid;    // |T_fe_mid - T_exact_mid| (N values)
    double max_midpoint_error;        // max of error_mid

    nlohmann::json to_json() const;
};

// Result of the full mesh convergence study
struct MeshStudyResult {
    RodParameters params;
    std::vector<FEASolution> solutions;  // One per mesh density
    std::string csv_export;              // Pre-formatted CSV string

    nlohmann::json to_json() const;
};

class Solver1D {
public:
    // Exact analytical solution: T(x) = T_left + (T_right - T_left)(x/L) + (q/(2k))*x*(L - x)
    static double exact_solution(double x, const RodParameters& p);

    // Solve the 1D steady-state FEA problem for a given number of elements N
    static core::Result<FEASolution> solve(const RodParameters& params, size_t N);

    // Run mesh convergence study for N = {10, 20, 40, 80}
    static core::Result<MeshStudyResult> mesh_study(
        const RodParameters& params,
        const std::vector<size_t>& mesh_sizes = {10, 20, 40, 80}
    );
};

} // namespace enginex::thermal

