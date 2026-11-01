#include "workbench/thermal/solver_1d.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace enginex::thermal {

// ── RodParameters serialization ─────────────────────────────────────────────

nlohmann::json RodParameters::to_json() const {
    return nlohmann::json{
        {"L", L},
        {"A", A},
        {"k", k},
        {"q", q},
        {"T_left", T_left},
        {"T_right", T_right}
    };
}

// ── FEASolution serialization ───────────────────────────────────────────────

nlohmann::json FEASolution::to_json() const {
    return nlohmann::json{
        {"N", N},
        {"h", h},
        {"x_nodes", x_nodes},
        {"T_fe", T_fe},
        {"T_exact", T_exact},
        {"x_midpoints", x_midpoints},
        {"T_fe_mid", T_fe_mid},
        {"T_exact_mid", T_exact_mid},
        {"error_mid", error_mid},
        {"max_midpoint_error", max_midpoint_error}
    };
}

// ── MeshStudyResult serialization ───────────────────────────────────────────

nlohmann::json MeshStudyResult::to_json() const {
    nlohmann::json j;
    j["params"] = params.to_json();
    j["csv_export"] = csv_export;

    nlohmann::json sols = nlohmann::json::array();
    for (const auto& s : solutions) {
        sols.push_back(s.to_json());
    }
    j["solutions"] = sols;

    return j;
}

// ── Exact analytical solution ───────────────────────────────────────────────

double Solver1D::exact_solution(double x, const RodParameters& p) {
    // T(x) = T_left + (T_right - T_left)*(x/L) + (q/(2*k))*x*(L - x)
    return p.T_left
         + (p.T_right - p.T_left) * (x / p.L)
         + (p.q / (2.0 * p.k)) * x * (p.L - x);
}

core::Result<void> Solver1D::validate_inputs(
    const RodParameters& params,
    const std::vector<size_t>& mesh_sizes)
{
    if (!std::isfinite(params.L) || params.L <= 0.0) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Rod length L must be a positive finite real number");
    }
    if (!std::isfinite(params.A) || params.A <= 0.0) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Cross-sectional area A must be a positive finite real number");
    }
    if (!std::isfinite(params.k) || params.k <= 0.0) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Thermal conductivity k must be a positive finite real number");
    }
    if (!std::isfinite(params.q)) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Volumetric heat generation rate q must be finite");
    }
    if (!std::isfinite(params.T_left) || params.T_left < -273.15) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Left boundary temperature T_left must be finite and at or above absolute zero (-273.15 C)");
    }
    if (!std::isfinite(params.T_right) || params.T_right < -273.15) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Right boundary temperature T_right must be finite and at or above absolute zero (-273.15 C)");
    }

    if (mesh_sizes.empty()) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "mesh_sizes must contain at least one mesh");
    }
    if (mesh_sizes.size() > kMaxMeshStudySizes) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "mesh_sizes must contain no more than " + std::to_string(kMaxMeshStudySizes) + " meshes");
    }

    const double two_k = 2.0 * params.k;
    const double source_coefficient = params.q / two_k;
    const double temperature_delta = params.T_right - params.T_left;
    if (!std::isfinite(two_k) || !std::isfinite(source_coefficient) ||
        !std::isfinite(temperature_delta) ||
        !std::isfinite(source_coefficient * params.L)) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Thermal parameters produce non-finite intermediate values");
    }

    const double maximum_source_term =
        (source_coefficient * (params.L / 2.0)) * (params.L / 2.0);
    if (!std::isfinite(maximum_source_term) ||
        !std::isfinite(params.T_left + maximum_source_term) ||
        !std::isfinite(params.T_right + maximum_source_term)) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Thermal parameters produce non-finite temperatures");
    }

    const double stiffness_numerator = params.k * params.A;
    const double load_numerator = params.q * params.A;
    if (!std::isfinite(stiffness_numerator) || stiffness_numerator <= 0.0 ||
        !std::isfinite(load_numerator)) {
        return core::Result<void>::fail(
            core::StatusCode::InvalidArgument,
            "Thermal parameters produce non-finite element coefficients");
    }

    for (const size_t N : mesh_sizes) {
        if (N < 2) {
            return core::Result<void>::fail(
                core::StatusCode::InvalidArgument,
                "Number of elements N must be at least 2");
        }
        if (N > kMaxMeshElements) {
            return core::Result<void>::fail(
                core::StatusCode::InvalidArgument,
                "Number of elements N must not exceed " + std::to_string(kMaxMeshElements));
        }

        const double h = params.L / static_cast<double>(N);
        const double stiffness_coefficient = stiffness_numerator / h;
        const double load_coefficient = (load_numerator * h) / 2.0;
        if (!std::isfinite(h) || h <= 0.0 ||
            !std::isfinite(stiffness_coefficient) || stiffness_coefficient <= 0.0 ||
            !std::isfinite(load_coefficient) ||
            !std::isfinite(2.0 * stiffness_coefficient) ||
            !std::isfinite(2.0 * load_coefficient)) {
            return core::Result<void>::fail(
                core::StatusCode::InvalidArgument,
                "Thermal parameters produce non-finite solver coefficients for N=" + std::to_string(N));
        }
    }

    return core::Result<void>::ok();
}

// ── FEA solver (single mesh) ────────────────────────────────────────────────

core::Result<FEASolution> Solver1D::solve(const RodParameters& params, size_t N) {
    // ── Input validation ────────────────────────────────────────────────
    auto validation = validate_inputs(params, {N});
    if (validation.is_err()) {
        return core::Result<FEASolution>::fail(
            validation.error().code, validation.error().message);
    }

    const size_t n_nodes = N + 1;
    const double h = params.L / static_cast<double>(N);

    // ── Nodal coordinates ───────────────────────────────────────────────
    std::vector<double> x_nodes(n_nodes);
    for (size_t i = 0; i < n_nodes; ++i) {
        x_nodes[i] = static_cast<double>(i) * h;
    }

    // ── Assemble global stiffness matrix K (flattened row-major) ────────
    // K is (n_nodes × n_nodes), tridiagonal from 2-node element assembly
    std::vector<double> K(n_nodes * n_nodes, 0.0);
    std::vector<double> F(n_nodes, 0.0);

    const double ke_coeff = (params.k * params.A) / h;   // k*A/h
    const double fe_coeff = (params.q * params.A * h) / 2.0;  // q*A*h/2

    for (size_t e = 0; e < N; ++e) {
        // Element e connects node e and node e+1
        // Element stiffness: ke_coeff * [[1, -1], [-1, 1]]
        K[e * n_nodes + e]           += ke_coeff;         // K[e][e]
        K[e * n_nodes + (e + 1)]     += -ke_coeff;        // K[e][e+1]
        K[(e + 1) * n_nodes + e]     += -ke_coeff;        // K[e+1][e]
        K[(e + 1) * n_nodes + (e + 1)] += ke_coeff;       // K[e+1][e+1]

        // Consistent element load: fe_coeff * [1, 1]^T
        F[e]     += fe_coeff;
        F[e + 1] += fe_coeff;
    }

    // ── Apply Dirichlet BCs (full elimination) ──────────────────────────
    // Prescribed nodes: node 0 (T_left), node N (T_right)
    auto apply_dirichlet = [&](size_t p, double T_p) {
        // Modify RHS for all free rows
        for (size_t i = 0; i < n_nodes; ++i) {
            if (i != p) {
                F[i] -= K[i * n_nodes + p] * T_p;
            }
        }
        // Zero out row p and column p
        for (size_t j = 0; j < n_nodes; ++j) {
            K[p * n_nodes + j] = 0.0;  // row p
            K[j * n_nodes + p] = 0.0;  // column p
        }
        // Set diagonal and RHS
        K[p * n_nodes + p] = 1.0;
        F[p] = T_p;
    };

    apply_dirichlet(0, params.T_left);
    apply_dirichlet(N, params.T_right);

    // ── Thomas algorithm for tridiagonal system ─────────────────────────
    // Extract diagonals from the full matrix
    std::vector<double> a(n_nodes, 0.0);  // lower diagonal (a[0] unused)
    std::vector<double> b(n_nodes, 0.0);  // main diagonal
    std::vector<double> c(n_nodes, 0.0);  // upper diagonal (c[n_nodes-1] unused)
    std::vector<double> d = F;            // RHS copy

    for (size_t i = 0; i < n_nodes; ++i) {
        b[i] = K[i * n_nodes + i];
        if (i > 0)             a[i] = K[i * n_nodes + (i - 1)];
        if (i < n_nodes - 1)   c[i] = K[i * n_nodes + (i + 1)];
    }

    // Forward sweep
    for (size_t i = 1; i < n_nodes; ++i) {
        double m = a[i] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
    }

    // Back substitution
    std::vector<double> T_fe(n_nodes);
    T_fe[n_nodes - 1] = d[n_nodes - 1] / b[n_nodes - 1];
    for (size_t i = n_nodes - 2; ; --i) {
        T_fe[i] = (d[i] - c[i] * T_fe[i + 1]) / b[i];
        if (i == 0) break;  // avoid underflow on unsigned
    }

    // ── Compute exact solution at nodes ─────────────────────────────────
    std::vector<double> T_exact(n_nodes);
    for (size_t i = 0; i < n_nodes; ++i) {
        T_exact[i] = exact_solution(x_nodes[i], params);
    }

    // ── Compute midpoint quantities ─────────────────────────────────────
    std::vector<double> x_midpoints(N);
    std::vector<double> T_fe_mid(N);
    std::vector<double> T_exact_mid(N);
    std::vector<double> error_mid(N);

    double max_midpoint_error = 0.0;
    for (size_t e = 0; e < N; ++e) {
        x_midpoints[e]  = (x_nodes[e] + x_nodes[e + 1]) / 2.0;
        T_fe_mid[e]     = (T_fe[e] + T_fe[e + 1]) / 2.0;
        T_exact_mid[e]  = exact_solution(x_midpoints[e], params);
        error_mid[e]    = std::fabs(T_fe_mid[e] - T_exact_mid[e]);
        max_midpoint_error = std::fmax(max_midpoint_error, error_mid[e]);
    }

    // ── Build result ────────────────────────────────────────────────────
    FEASolution sol;
    sol.N                  = N;
    sol.h                  = h;
    sol.x_nodes            = std::move(x_nodes);
    sol.T_fe               = std::move(T_fe);
    sol.T_exact            = std::move(T_exact);
    sol.x_midpoints        = std::move(x_midpoints);
    sol.T_fe_mid           = std::move(T_fe_mid);
    sol.T_exact_mid        = std::move(T_exact_mid);
    sol.error_mid          = std::move(error_mid);
    sol.max_midpoint_error = max_midpoint_error;

    return core::Result<FEASolution>::ok(std::move(sol));
}

// ── Mesh convergence study ──────────────────────────────────────────────────

core::Result<MeshStudyResult> Solver1D::mesh_study(
    const RodParameters& params,
    const std::vector<size_t>& mesh_sizes,
    std::function<bool()> is_cancelled,
    std::function<void(float, const std::string&)> on_progress)
{
    auto validation = validate_inputs(params, mesh_sizes);
    if (validation.is_err()) {
        return core::Result<MeshStudyResult>::fail(
            validation.error().code, validation.error().message);
    }

    MeshStudyResult study;
    study.params = params;

    // CSV header
    std::ostringstream csv;
    csv << "N,h,max_midpoint_error_C,node_0_T_C,node_mid_T_C,node_N_T_C\n";
    csv << std::setprecision(12);

    const size_t total_steps = mesh_sizes.size();

    for (size_t i = 0; i < total_steps; ++i) {
        if (is_cancelled && is_cancelled()) {
            return core::Result<MeshStudyResult>::fail(
                core::StatusCode::Cancelled, "Thermal analysis cancelled by user");
        }

        size_t N = mesh_sizes[i];

        if (on_progress) {
            float progress = static_cast<float>(i) / static_cast<float>(total_steps);
            on_progress(progress, "Solving FEA mesh with N=" + std::to_string(N));
        }

        auto result = solve(params, N);
        if (result.is_err()) {
            return core::Result<MeshStudyResult>::fail(
                result.error().code, result.error().message);
        }

        const auto& sol = result.value();

        csv << sol.N << ","
            << sol.h << ","
            << sol.max_midpoint_error << ","
            << sol.T_fe[0] << ","
            << sol.T_fe[sol.N / 2] << ","
            << sol.T_fe[sol.N] << "\n";

        study.solutions.push_back(std::move(result.value()));

        if (is_cancelled && is_cancelled()) {
            return core::Result<MeshStudyResult>::fail(
                core::StatusCode::Cancelled, "Thermal analysis cancelled by user");
        }
    }

    if (on_progress) {
        on_progress(1.0f, "Thermal analysis complete");
    }

    study.csv_export = csv.str();
    return core::Result<MeshStudyResult>::ok(std::move(study));
}

} // namespace enginex::thermal
