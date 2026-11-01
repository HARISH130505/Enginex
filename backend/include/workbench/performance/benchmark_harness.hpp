#pragma once

#include <vector>
#include <string>
#include <cstddef>
#include <cstdint>
#include <functional>
#include "workbench/core/result.hpp"
#include "workbench/core/json.hpp"
#include "workbench/thermal/solver_1d.hpp"
#include "workbench/performance/batch_generator.hpp"

namespace enginex::performance {

struct BenchmarkConfig {
    size_t batch_size = 100;
    std::vector<size_t> worker_counts = {1, 2, 3, 4};
    size_t warmup_reps = 1;
    size_t timed_reps = 5;
    uint32_t seed = 42;
    size_t elements_per_sim = 40;

    nlohmann::json to_json() const;
    static BenchmarkConfig from_json(const nlohmann::json& j);
};

struct WorkerBenchmarkResult {
    size_t workers = 1;
    std::vector<double> rep_durations_ms;  // Timings for each timed rep
    double median_ms = 0.0;                // Median duration
    double speedup = 1.0;                  // S_p = T_1 / T_p
    double efficiency = 1.0;               // E_p = S_p / p
    double max_temp_diff_vs_serial = 0.0;  // max |T_p - T_1| across all nodes & cases

    nlohmann::json to_json() const;
};

struct BenchmarkResult {
    std::string hardware_model;
    size_t physical_cores = 0;
    size_t logical_cores = 0;
    std::string ram_capacity;
    std::string os_version;
    std::string compiler_version;
    std::string cpp_standard;
    std::string build_config;
    std::string compiler_flags;
    std::string timing_methodology;

    size_t batch_size = 0;
    size_t elements_per_sim = 40;
    uint32_t seed = 42;
    size_t warmup_reps = 1;
    size_t timed_reps = 5;

    std::vector<WorkerBenchmarkResult> worker_results;
    double overall_max_discrepancy = 0.0;
    bool agreement_verified = false;  // true if max diff <= 1e-8 °C
    std::string csv_export;

    nlohmann::json to_json() const;
};

class BenchmarkHarness {
public:
    // Run batch simulation with specified number of workers.
    // Results are guaranteed to be in the exact original input index order.
    static std::vector<thermal::FEASolution> run_batch(
        const std::vector<SimulationInput>& batch,
        size_t num_workers
    );

    // Run complete benchmark suite with warmup, timed reps, speedup, efficiency,
    // and numerical agreement verification across serial and parallel workers.
    static core::Result<BenchmarkResult> run_benchmark(
        const BenchmarkConfig& config = BenchmarkConfig{},
        std::function<bool()> is_cancelled = nullptr,
        std::function<void(float, const std::string&)> on_progress = nullptr
    );
};

} // namespace enginex::performance

