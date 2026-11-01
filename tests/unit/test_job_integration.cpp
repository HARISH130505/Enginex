#include "workbench/core/job_manager.hpp"
#include "workbench/thermal/solver_1d.hpp"
#include "workbench/performance/benchmark_harness.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <set>
#include <atomic>
#include <vector>

using namespace enginex::core;
using namespace enginex::thermal;
using namespace enginex::performance;

// Helper: wait for a job to reach a terminal or target state
static bool wait_for_status(const std::string& job_id, const std::string& target_status, int timeout_ms = 30000) {
    auto& jm = JobManager::instance();
    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto snap = jm.get_job(job_id);
        if (snap.is_ok()) {
            std::string status = snap.value()["status"].get<std::string>();
            if (status == target_status) return true;
            if (status == "COMPLETED" || status == "FAILED" || status == "CANCELLED") {
                return status == target_status;
            }
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        if (elapsed > timeout_ms) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

static bool wait_for_terminal(const std::string& job_id, int timeout_ms = 30000) {
    auto& jm = JobManager::instance();
    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto snap = jm.get_job(job_id);
        if (snap.is_ok()) {
            std::string status = snap.value()["status"].get<std::string>();
            if (status == "COMPLETED" || status == "FAILED" || status == "CANCELLED") return true;
        }
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        if (elapsed > timeout_ms) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

// ============================================================================
// Test 1: Job submission returns unique ID in QUEUED state
// ============================================================================
void test_job_submission_returns_queued() {
    std::cout << "[RUN] Test 1: Job submission returns unique ID in QUEUED state\n";
    auto& jm = JobManager::instance();
    jm.reset();

    auto res = jm.submit_job("test", [](const std::string&) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
    assert(res.is_ok());
    std::string job_id = res.value();
    assert(!job_id.empty());
    assert(job_id.substr(0, 4) == "job_");

    // Should be QUEUED or already RUNNING (worker picked it up quickly)
    auto snap = jm.get_job(job_id);
    assert(snap.is_ok());
    std::string status = snap.value()["status"].get<std::string>();
    assert(status == "QUEUED" || status == "RUNNING");

    wait_for_terminal(job_id);
    std::cout << "[PASS] Test 1 passed.\n";
}

// ============================================================================
// Test 2: Transition from QUEUED to RUNNING to COMPLETED
// ============================================================================
void test_queued_to_running_to_completed() {
    std::cout << "[RUN] Test 2: QUEUED -> RUNNING -> COMPLETED transition\n";
    auto& jm = JobManager::instance();
    jm.reset();

    std::atomic<bool> start_flag{false};
    auto res = jm.submit_job("test_lifecycle", [&start_flag](const std::string& job_id) {
        while (!start_flag.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        JobManager::instance().update_progress(job_id, 0.5f, "Halfway");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        JobManager::instance().mark_completed(job_id, "{\"value\": 42}");
    });
    assert(res.is_ok());
    std::string job_id = res.value();

    // Wait until RUNNING
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    start_flag.store(true);

    bool reached = wait_for_status(job_id, "COMPLETED");
    assert(reached);

    auto snap = jm.get_job(job_id);
    assert(snap.is_ok());
    assert(snap.value()["status"] == "COMPLETED");
    assert(snap.value()["progress"] == 1.0f);
    assert(snap.value()["result"]["value"] == 42);
    std::cout << "[PASS] Test 2 passed.\n";
}

// ============================================================================
// Test 3: Failed job and error reporting
// ============================================================================
void test_failed_job() {
    std::cout << "[RUN] Test 3: Failed job and error reporting\n";
    auto& jm = JobManager::instance();
    jm.reset();

    auto res = jm.submit_job("test_fail", [](const std::string&) {
        throw std::runtime_error("Simulated computation failure");
    });
    assert(res.is_ok());
    std::string job_id = res.value();

    bool reached = wait_for_terminal(job_id);
    assert(reached);

    auto snap = jm.get_job(job_id);
    assert(snap.is_ok());
    assert(snap.value()["status"] == "FAILED");
    assert(snap.value().contains("error_message"));
    std::string err_msg = snap.value()["error_message"].get<std::string>();
    assert(err_msg.find("Simulated computation failure") != std::string::npos);
    std::cout << "[PASS] Test 3 passed.\n";
}

// ============================================================================
// Test 4: Cancellation while queued
// ============================================================================
void test_cancel_while_queued() {
    std::cout << "[RUN] Test 4: Cancellation while queued\n";
    auto& jm = JobManager::instance();
    jm.reset();

    // Create job manually (not through submit) to test cancel while truly queued
    std::string job_id = jm.create_job("test_cancel_queued", "Queued for cancel");
    auto cancel_res = jm.request_cancel(job_id);
    assert(cancel_res.is_ok());

    auto snap = jm.get_job(job_id);
    assert(snap.is_ok());
    assert(snap.value()["status"] == "CANCELLED");
    std::cout << "[PASS] Test 4 passed.\n";
}

// ============================================================================
// Test 5: Cancellation while running (partial results NOT stored as completed)
// ============================================================================
void test_cancel_while_running() {
    std::cout << "[RUN] Test 5: Cancellation while running\n";
    auto& jm = JobManager::instance();
    jm.reset();

    std::atomic<bool> task_started{false};
    auto res = jm.submit_job("test_cancel_running", [&task_started](const std::string& job_id) {
        task_started.store(true);
        auto& mgr = JobManager::instance();
        mgr.update_progress(job_id, 0.1f, "Starting long work");

        for (int i = 0; i < 100; ++i) {
            if (mgr.is_cancel_requested(job_id)) {
                mgr.mark_cancelled(job_id, "Cancelled during computation");
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        mgr.mark_completed(job_id, "{\"partial\": true}");
    });
    assert(res.is_ok());
    std::string job_id = res.value();

    // Wait for task to start
    while (!task_started.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto cancel_res = jm.request_cancel(job_id);
    assert(cancel_res.is_ok());

    bool reached = wait_for_terminal(job_id);
    assert(reached);

    auto snap = jm.get_job(job_id);
    assert(snap.is_ok());
    assert(snap.value()["status"] == "CANCELLED");
    // Partial results must NOT be stored as completed
    assert(snap.value()["result"].is_null());
    std::cout << "[PASS] Test 5 passed.\n";
}

// ============================================================================
// Test 6: Cancellation after completion returns error
// ============================================================================
void test_cancel_after_completion() {
    std::cout << "[RUN] Test 6: Cancellation after completion returns error\n";
    auto& jm = JobManager::instance();
    jm.reset();

    auto res = jm.submit_job("test_cancel_after", [](const std::string& job_id) {
        JobManager::instance().mark_completed(job_id, "{\"done\": true}");
    });
    assert(res.is_ok());
    std::string job_id = res.value();

    wait_for_terminal(job_id);

    auto cancel_res = jm.request_cancel(job_id);
    assert(cancel_res.is_err());
    assert(cancel_res.error().code == StatusCode::InvalidArgument);
    std::cout << "[PASS] Test 6 passed.\n";
}

// ============================================================================
// Test 7: Unknown job ID returns NotFound
// ============================================================================
void test_unknown_job_id() {
    std::cout << "[RUN] Test 7: Unknown job ID returns NotFound\n";
    auto& jm = JobManager::instance();

    auto snap = jm.get_job("job_nonexistent_999999");
    assert(snap.is_err());
    assert(snap.error().code == StatusCode::NotFound);

    auto res_result = jm.get_job_result("job_nonexistent_999999");
    assert(res_result.is_err());
    assert(res_result.error().code == StatusCode::NotFound);

    auto cancel_res = jm.request_cancel("job_nonexistent_999999");
    assert(cancel_res.is_err());
    assert(cancel_res.error().code == StatusCode::NotFound);
    std::cout << "[PASS] Test 7 passed.\n";
}

// ============================================================================
// Test 8: Result retrieval before completion returns error
// ============================================================================
void test_result_before_completion() {
    std::cout << "[RUN] Test 8: Result retrieval before completion returns error\n";
    auto& jm = JobManager::instance();
    jm.reset();

    std::atomic<bool> hold{true};
    auto res = jm.submit_job("test_result_early", [&hold](const std::string& job_id) {
        while (hold.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        JobManager::instance().mark_completed(job_id, "{\"v\": 1}");
    });
    assert(res.is_ok());
    std::string job_id = res.value();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto result_res = jm.get_job_result(job_id);
    assert(result_res.is_err());
    assert(result_res.error().code == StatusCode::Unavailable);

    hold.store(false);
    wait_for_terminal(job_id);
    std::cout << "[PASS] Test 8 passed.\n";
}

// ============================================================================
// Test 9: Result retrieval after completion matches
// ============================================================================
void test_result_after_completion() {
    std::cout << "[RUN] Test 9: Result retrieval after completion\n";
    auto& jm = JobManager::instance();
    jm.reset();

    auto res = jm.submit_job("test_result_after", [](const std::string& job_id) {
        nlohmann::json result = {{"pi", 3.14159}, {"count", 100}};
        JobManager::instance().mark_completed(job_id, result.dump());
    });
    assert(res.is_ok());
    std::string job_id = res.value();

    wait_for_terminal(job_id);

    auto result_res = jm.get_job_result(job_id);
    assert(result_res.is_ok());
    assert(result_res.value()["pi"].get<double>() > 3.14);
    assert(result_res.value()["count"] == 100);
    std::cout << "[PASS] Test 9 passed.\n";
}

// ============================================================================
// Test 10: Concurrent job submissions
// ============================================================================
void test_concurrent_submissions() {
    std::cout << "[RUN] Test 10: Concurrent job submissions\n";
    auto& jm = JobManager::instance();
    jm.reset();

    const int N = 8;
    std::vector<std::string> job_ids;
    std::set<std::string> unique_ids;

    for (int i = 0; i < N; ++i) {
        auto res = jm.submit_job("concurrent_" + std::to_string(i),
            [i](const std::string& job_id) {
                nlohmann::json r = {{"index", i}};
                JobManager::instance().mark_completed(job_id, r.dump());
            }
        );
        assert(res.is_ok());
        job_ids.push_back(res.value());
        unique_ids.insert(res.value());
    }

    // All IDs must be unique
    assert(unique_ids.size() == static_cast<size_t>(N));

    // Wait for all to finish
    for (auto& jid : job_ids) {
        bool done = wait_for_terminal(jid);
        assert(done);
    }

    // Verify all completed
    for (int i = 0; i < N; ++i) {
        auto snap = jm.get_job(job_ids[i]);
        assert(snap.is_ok());
        assert(snap.value()["status"] == "COMPLETED");
    }
    std::cout << "[PASS] Test 10 passed.\n";
}

// ============================================================================
// Test 11: Thermal FEA result correctness via async execution
// ============================================================================
void test_thermal_async_correctness() {
    std::cout << "[RUN] Test 11: Thermal FEA async result correctness\n";
    auto& jm = JobManager::instance();
    jm.reset();

    RodParameters params;
    params.L = 1.0; params.A = 1e-4; params.k = 10.0;
    params.q = 10000.0; params.T_left = 100.0; params.T_right = 20.0;
    std::vector<size_t> mesh_sizes = {10, 20};

    // Compute expected result synchronously
    auto sync_result = Solver1D::mesh_study(params, mesh_sizes);
    assert(sync_result.is_ok());
    auto expected_json = sync_result.value().to_json();

    // Submit async job
    auto res = jm.submit_job("thermal",
        [params, mesh_sizes](const std::string& job_id) {
            auto result = Solver1D::mesh_study(
                params, mesh_sizes,
                [job_id]() { return JobManager::instance().is_cancel_requested(job_id); },
                [job_id](float p, const std::string& msg) {
                    JobManager::instance().update_progress(job_id, p, msg);
                }
            );
            if (result.is_ok()) {
                JobManager::instance().mark_completed(job_id, result.value().to_json().dump());
            } else {
                JobManager::instance().mark_failed(job_id, result.error().message);
            }
        }
    );
    assert(res.is_ok());
    std::string job_id = res.value();

    bool done = wait_for_terminal(job_id);
    assert(done);

    auto snap = jm.get_job(job_id);
    assert(snap.is_ok());
    assert(snap.value()["status"] == "COMPLETED");

    auto async_result = jm.get_job_result(job_id);
    assert(async_result.is_ok());

    // Compare key fields
    auto& async_json = async_result.value();
    assert(async_json["solutions"].size() == expected_json["solutions"].size());
    for (size_t i = 0; i < expected_json["solutions"].size(); ++i) {
        double expected_err = expected_json["solutions"][i]["max_midpoint_error"].get<double>();
        double async_err = async_json["solutions"][i]["max_midpoint_error"].get<double>();
        assert(std::fabs(expected_err - async_err) < 1e-12);
    }
    std::cout << "[PASS] Test 11 passed.\n";
}

// ============================================================================
// Test 12: Benchmark result correctness via async execution
// ============================================================================
void test_benchmark_async_correctness() {
    std::cout << "[RUN] Test 12: Benchmark async result correctness\n";
    auto& jm = JobManager::instance();
    jm.reset();

    BenchmarkConfig config;
    config.batch_size = 10;
    config.worker_counts = {1, 2};
    config.warmup_reps = 0;
    config.timed_reps = 1;
    config.seed = 42;
    config.elements_per_sim = 10;

    // Submit async job
    auto res = jm.submit_job("performance",
        [config](const std::string& job_id) {
            auto result = BenchmarkHarness::run_benchmark(
                config,
                [job_id]() { return JobManager::instance().is_cancel_requested(job_id); },
                [job_id](float p, const std::string& msg) {
                    JobManager::instance().update_progress(job_id, p, msg);
                }
            );
            if (result.is_ok()) {
                JobManager::instance().mark_completed(job_id, result.value().to_json().dump());
            } else {
                JobManager::instance().mark_failed(job_id, result.error().message);
            }
        }
    );
    assert(res.is_ok());
    std::string job_id = res.value();

    bool done = wait_for_terminal(job_id, 60000);
    assert(done);

    auto snap = jm.get_job(job_id);
    assert(snap.is_ok());
    assert(snap.value()["status"] == "COMPLETED");

    auto async_result = jm.get_job_result(job_id);
    assert(async_result.is_ok());
    auto& r = async_result.value();
    assert(r.contains("worker_results"));
    assert(r["worker_results"].size() == 2);
    assert(r["agreement_verified"] == true);
    std::cout << "[PASS] Test 12 passed.\n";
}

// ============================================================================
// Test 13: Bounded pool queue capacity
// ============================================================================
void test_bounded_pool_capacity() {
    std::cout << "[RUN] Test 13: Bounded pool queue capacity\n";
    auto& jm = JobManager::instance();
    jm.reset();

    // The default pool has max_queue_size=64. Submitting more than that should
    // eventually return ResourceExhausted (but workers drain quickly).
    // Test: submit many fast jobs and verify they all complete.
    const int N = 20;
    std::vector<std::string> ids;
    for (int i = 0; i < N; ++i) {
        auto res = jm.submit_job("bounded_" + std::to_string(i),
            [i](const std::string& job_id) {
                nlohmann::json r = {{"idx", i}};
                JobManager::instance().mark_completed(job_id, r.dump());
            }
        );
        assert(res.is_ok());
        ids.push_back(res.value());
    }

    for (auto& jid : ids) {
        bool done = wait_for_terminal(jid);
        assert(done);
        auto snap = jm.get_job(jid);
        assert(snap.is_ok());
        assert(snap.value()["status"] == "COMPLETED");
    }
    std::cout << "[PASS] Test 13 passed.\n";
}

// ============================================================================
// Test 14: Cancelled job result is not retrievable
// ============================================================================
void test_cancelled_job_result() {
    std::cout << "[RUN] Test 14: Cancelled job result is not retrievable\n";
    auto& jm = JobManager::instance();
    jm.reset();

    std::string job_id = jm.create_job("cancel_result_test", "Testing");
    jm.request_cancel(job_id);

    auto result = jm.get_job_result(job_id);
    assert(result.is_err());
    assert(result.error().code == StatusCode::Cancelled);
    std::cout << "[PASS] Test 14 passed.\n";
}

// ============================================================================
// Test 15: Failed job result returns InternalError
// ============================================================================
void test_failed_job_result() {
    std::cout << "[RUN] Test 15: Failed job result returns InternalError\n";
    auto& jm = JobManager::instance();
    jm.reset();

    auto res = jm.submit_job("fail_result_test", [](const std::string&) {
        throw std::runtime_error("Intentional failure for test");
    });
    assert(res.is_ok());
    std::string job_id = res.value();

    wait_for_terminal(job_id);

    auto result = jm.get_job_result(job_id);
    assert(result.is_err());
    assert(result.error().code == StatusCode::InternalError);
    std::cout << "[PASS] Test 15 passed.\n";
}

// ============================================================================
int main() {
    std::cout << "==================================================\n";
    std::cout << "  ENGINEX Job Integration Tests\n";
    std::cout << "==================================================\n";

    try {
        test_job_submission_returns_queued();
        test_queued_to_running_to_completed();
        test_failed_job();
        test_cancel_while_queued();
        test_cancel_while_running();
        test_cancel_after_completion();
        test_unknown_job_id();
        test_result_before_completion();
        test_result_after_completion();
        test_concurrent_submissions();
        test_thermal_async_correctness();
        test_benchmark_async_correctness();
        test_bounded_pool_capacity();
        test_cancelled_job_result();
        test_failed_job_result();

        std::cout << "\n>>> ALL 15 JOB INTEGRATION TESTS PASSED SUCCESSFULLY! <<<\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Unexpected exception: " << ex.what() << "\n";
        return 1;
    }
}

