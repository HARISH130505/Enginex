#include "workbench/core/result.hpp"
#include "workbench/core/job_manager.hpp"
#include <iostream>
#include <cassert>

using namespace enginex::core;

void test_result_type() {
    std::cout << "[RUN] Testing Result<T> monad...\n";

    auto ok_res = Result<int>::ok(42);
    assert(ok_res.is_ok());
    assert(!ok_res.is_err());
    assert(ok_res.value() == 42);

    auto err_res = Result<int>::fail(StatusCode::InvalidArgument, "Invalid parameter");
    assert(!err_res.is_ok());
    assert(err_res.is_err());
    assert(err_res.error().code == StatusCode::InvalidArgument);
    assert(err_res.error().message == "Invalid parameter");

    auto void_ok = Result<void>::ok();
    assert(void_ok.is_ok());

    auto void_fail = Result<void>::fail(StatusCode::Timeout, "Operation timed out");
    assert(void_fail.is_err());
    assert(void_fail.error().code == StatusCode::Timeout);

    std::cout << "[PASS] Result<T> monad tests passed.\n";
}

void test_job_manager_lifecycle() {
    std::cout << "[RUN] Testing JobManager lifecycle...\n";

    auto& jm = JobManager::instance();
    std::string job_id = jm.create_job("TEST_TASK", "Starting task");
    assert(!job_id.empty());

    auto snapshot1 = jm.get_job(job_id);
    assert(snapshot1.is_ok());
    assert(snapshot1.value()["status"] == "QUEUED" || snapshot1.value()["status"] == "PENDING");
    assert(snapshot1.value()["progress"] == 0.0f);

    // Update progress
    jm.update_progress(job_id, 0.45f, "Halfway done");
    auto snapshot2 = jm.get_job(job_id);
    assert(snapshot2.value()["status"] == "RUNNING");
    assert(snapshot2.value()["progress"] == 0.45f);
    assert(snapshot2.value()["status_message"] == "Halfway done");

    // Complete job
    nlohmann::json test_result = {{"computed_value", 3.14159}};
    jm.mark_completed(job_id, test_result.dump(), "Computation finished");
    auto snapshot3 = jm.get_job(job_id);
    assert(snapshot3.value()["status"] == "COMPLETED");
    assert(snapshot3.value()["progress"] == 1.0f);
    assert(snapshot3.value()["result"]["computed_value"] == 3.14159);

    std::cout << "[PASS] JobManager lifecycle tests passed.\n";
}

void test_job_cancellation() {
    std::cout << "[RUN] Testing JobManager cancellation...\n";

    auto& jm = JobManager::instance();
    std::string job_id = jm.create_job("LONG_TASK", "Executing lengthy simulation");

    assert(!jm.is_cancel_requested(job_id));

    auto cancel_res = jm.request_cancel(job_id);
    assert(cancel_res.is_ok());
    assert(jm.is_cancel_requested(job_id));

    jm.mark_cancelled(job_id, "User requested cancellation");
    auto snapshot = jm.get_job(job_id);
    assert(snapshot.value()["status"] == "CANCELLED");
    assert(snapshot.value()["cancel_requested"] == true);

    std::cout << "[PASS] JobManager cancellation tests passed.\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "  ENGINEX Skeleton Unit Tests (C++17)\n";
    std::cout << "==================================================\n";

    try {
        test_result_type();
        test_job_manager_lifecycle();
        test_job_cancellation();
        std::cout << "\n>>> ALL PHASE G1 CORE UNIT TESTS PASSED SUCCESSFULLY! <<<\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[FAIL] Unexpected exception: " << ex.what() << "\n";
        return 1;
    }
}

