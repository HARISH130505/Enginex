#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include "workbench/core/result.hpp"
#include "workbench/core/json.hpp"

namespace enginex::core {

enum class JobStatus {
    Pending = 0,
    Running,
    Completed,
    Cancelled,
    Failed
};

std::string job_status_to_string(JobStatus status);

struct JobInfo {
    std::string id;
    std::string type;
    JobStatus status;
    float progress; // 0.0 to 1.0
    std::string status_message;
    std::string result_json;
    std::string error_message;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point completed_at;
    std::atomic<bool> cancel_requested{false};

    nlohmann::json to_json() const;
};

class JobManager {
public:
    static JobManager& instance();

    // Create a new tracked job
    std::string create_job(const std::string& type, const std::string& initial_message = "Job initialized");

    // Check if cancellation has been requested for a job
    bool is_cancel_requested(const std::string& job_id);

    // Request cooperative cancellation of a job
    Result<void> request_cancel(const std::string& job_id);

    // Update progress and status
    void update_progress(const std::string& job_id, float progress, const std::string& message = "");

    // Set terminal state: completed
    void mark_completed(const std::string& job_id, const std::string& result_json, const std::string& final_message = "Complete");

    // Set terminal state: failed
    void mark_failed(const std::string& job_id, const std::string& error_message);

    // Set terminal state: cancelled
    void mark_cancelled(const std::string& job_id, const std::string& message = "Job was cancelled by user");

    // Get snapshot of a job's status
    Result<nlohmann::json> get_job(const std::string& job_id);

    // Clear completed/cancelled/failed jobs
    void clear_old_jobs();

private:
    JobManager() = default;
    ~JobManager() = default;
    JobManager(const JobManager&) = delete;
    JobManager& operator=(const JobManager&) = delete;

    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<JobInfo>> m_jobs;
    std::atomic<uint64_t> m_next_id{1000};
};

} // namespace enginex::core

