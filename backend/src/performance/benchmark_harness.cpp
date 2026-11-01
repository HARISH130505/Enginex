#include "workbench/performance/benchmark_harness.hpp"
#include "workbench/performance/thread_pool.hpp"
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <thread>
#include <cstdlib>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace enginex::performance {

// ── JSON Serialization ──────────────────────────────────────────────────────

nlohmann::json BenchmarkConfig::to_json() const {
    return nlohmann::json{
        {"batch_size", batch_size},
        {"worker_counts", worker_counts},
        {"warmup_reps", warmup_reps},
        {"timed_reps", timed_reps},
        {"seed", seed},
        {"elements_per_sim", elements_per_sim}
    };
}

BenchmarkConfig BenchmarkConfig::from_json(const nlohmann::json& j) {
    BenchmarkConfig cfg;
    if (j.contains("batch_size")) cfg.batch_size = j["batch_size"].get<size_t>();
    if (j.contains("worker_counts") && j["worker_counts"].is_array()) {
        cfg.worker_counts.clear();
        for (const auto& w : j["worker_counts"]) {
            cfg.worker_counts.push_back(w.get<size_t>());
        }
    }
    if (j.contains("warmup_reps")) cfg.warmup_reps = j["warmup_reps"].get<size_t>();
    if (j.contains("timed_reps")) cfg.timed_reps = j["timed_reps"].get<size_t>();
    if (j.contains("seed")) cfg.seed = j["seed"].get<uint32_t>();
    if (j.contains("elements_per_sim")) cfg.elements_per_sim = j["elements_per_sim"].get<size_t>();
    return cfg;
}

nlohmann::json WorkerBenchmarkResult::to_json() const {
    return nlohmann::json{
        {"workers", workers},
        {"rep_durations_ms", rep_durations_ms},
        {"median_ms", median_ms},
        {"speedup", speedup},
        {"efficiency", efficiency},
        {"max_temp_diff_vs_serial", max_temp_diff_vs_serial}
    };
}

nlohmann::json BenchmarkResult::to_json() const {
    nlohmann::json j;
    j["hardware_model"] = hardware_model;
    j["logical_cores"] = logical_cores;
    j["batch_size"] = batch_size;
    j["elements_per_sim"] = elements_per_sim;
    j["seed"] = seed;
    j["timed_reps"] = timed_reps;
    j["overall_max_discrepancy"] = overall_max_discrepancy;
    j["agreement_verified"] = agreement_verified;
    j["csv_export"] = csv_export;

    nlohmann::json wr = nlohmann::json::array();
    for (const auto& w : worker_results) {
        wr.push_back(w.to_json());
    }
    j["worker_results"] = wr;

    return j;
}

// ── Batch Execution ─────────────────────────────────────────────────────────

std::vector<thermal::FEASolution> BenchmarkHarness::run_batch(
    const std::vector<SimulationInput>& batch,
    size_t num_workers)
{
    const size_t n_jobs = batch.size();
    std::vector<thermal::FEASolution> results(n_jobs);

    if (num_workers <= 1) {
        // Serial execution
        for (size_t i = 0; i < n_jobs; ++i) {
            auto res = thermal::Solver1D::solve(batch[i].params, batch[i].N);
            if (res.is_ok()) {
                results[i] = std::move(res.value());
            }
        }
        return results;
    }

    // Parallel execution via ThreadPool
    ThreadPool pool(num_workers);

    for (size_t i = 0; i < n_jobs; ++i) {
        pool.enqueue([&batch, &results, i]() {
            auto res = thermal::Solver1D::solve(batch[i].params, batch[i].N);
            if (res.is_ok()) {
                results[i] = std::move(res.value());
            }
        });
    }

    pool.wait_all();
    return results;
}

// ── Benchmark Suite Execution ───────────────────────────────────────────────

static std::string detect_cpu_hardware() {
    const char* env_proc = std::getenv("PROCESSOR_IDENTIFIER");
    if (env_proc && env_proc[0] != '\0') {
        return std::string(env_proc);
    }
#ifdef _WIN32
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    std::ostringstream oss;
    oss << "Windows x64 Architecture " << sys_info.wProcessorArchitecture;
    return oss.str();
#else
    return "Host CPU Architecture";
#endif
}

core::Result<BenchmarkResult> BenchmarkHarness::run_benchmark(const BenchmarkConfig& config) {
    if (config.batch_size == 0) {
        return core::Result<BenchmarkResult>::fail(
            core::StatusCode::InvalidArgument, "Batch size must be > 0");
    }
    if (config.worker_counts.empty()) {
        return core::Result<BenchmarkResult>::fail(
            core::StatusCode::InvalidArgument, "worker_counts must not be empty");
    }
    if (config.timed_reps == 0) {
        return core::Result<BenchmarkResult>::fail(
            core::StatusCode::InvalidArgument, "timed_reps must be > 0");
    }

    BenchmarkResult bench;
    bench.hardware_model = detect_cpu_hardware();
    bench.logical_cores = std::thread::hardware_concurrency();
    bench.batch_size = config.batch_size;
    bench.elements_per_sim = config.elements_per_sim;
    bench.seed = config.seed;
    bench.timed_reps = config.timed_reps;

    // Generate deterministic batch
    auto batch = BatchGenerator::generate(
        config.batch_size,
        config.seed,
        config.elements_per_sim
    );

    // Compute baseline serial solutions to verify numerical agreement
    auto serial_solutions = run_batch(batch, 1);

    double t1_median = 0.0;
    double max_discrepancy = 0.0;

    // Run benchmarks for each worker count
    for (size_t p : config.worker_counts) {
        WorkerBenchmarkResult wr;
        wr.workers = p;

        // 1. Warmup runs
        for (size_t w = 0; w < config.warmup_reps; ++w) {
            auto warmup_res = run_batch(batch, p);
            (void)warmup_res;
        }

        // 2. Timed repetitions
        std::vector<thermal::FEASolution> last_parallel_sol;
        for (size_t r = 0; r < config.timed_reps; ++r) {
            auto start = std::chrono::high_resolution_clock::now();
            auto res = run_batch(batch, p);
            auto end = std::chrono::high_resolution_clock::now();

            double duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            wr.rep_durations_ms.push_back(duration_ms);

            if (r == config.timed_reps - 1) {
                last_parallel_sol = std::move(res);
            }
        }

        // 3. Compute median duration
        std::vector<double> sorted_times = wr.rep_durations_ms;
        std::sort(sorted_times.begin(), sorted_times.end());
        if (sorted_times.size() % 2 == 1) {
            wr.median_ms = sorted_times[sorted_times.size() / 2];
        } else {
            size_t mid = sorted_times.size() / 2;
            wr.median_ms = (sorted_times[mid - 1] + sorted_times[mid]) / 2.0;
        }

        if (p == 1 || t1_median == 0.0) {
            t1_median = wr.median_ms;
        }

        // 4. Compute Speedup and Efficiency
        wr.speedup = (wr.median_ms > 0.0) ? (t1_median / wr.median_ms) : 1.0;
        wr.efficiency = wr.speedup / static_cast<double>(p);

        // 5. Numerical agreement verification against serial
        double worker_max_diff = 0.0;
        for (size_t i = 0; i < config.batch_size && i < last_parallel_sol.size(); ++i) {
            const auto& s_ser = serial_solutions[i];
            const auto& s_par = last_parallel_sol[i];
            const size_t n_nodes = s_ser.T_fe.size();
            for (size_t j = 0; j < n_nodes && j < s_par.T_fe.size(); ++j) {
                double diff = std::fabs(s_ser.T_fe[j] - s_par.T_fe[j]);
                worker_max_diff = std::fmax(worker_max_diff, diff);
            }
        }

        wr.max_temp_diff_vs_serial = worker_max_diff;
        max_discrepancy = std::fmax(max_discrepancy, worker_max_diff);

        bench.worker_results.push_back(std::move(wr));
    }

    bench.overall_max_discrepancy = max_discrepancy;
    bench.agreement_verified = (max_discrepancy <= 1.0e-8);

    // 6. Generate CSV export
    std::ostringstream csv;
    csv << "workers,rep_1_ms,rep_2_ms,rep_3_ms,rep_4_ms,rep_5_ms,median_ms,speedup,efficiency,max_discrepancy_C\n";
    csv << std::setprecision(6) << std::fixed;

    for (const auto& wr : bench.worker_results) {
        csv << wr.workers << ",";
        for (size_t r = 0; r < 5; ++r) {
            if (r < wr.rep_durations_ms.size()) {
                csv << wr.rep_durations_ms[r] << ",";
            } else {
                csv << "0.000,";
            }
        }
        csv << wr.median_ms << ","
            << wr.speedup << ","
            << wr.efficiency << ","
            << std::scientific << wr.max_temp_diff_vs_serial << "\n"
            << std::fixed;
    }

    bench.csv_export = csv.str();
    return core::Result<BenchmarkResult>::ok(std::move(bench));
}

} // namespace enginex::performance

