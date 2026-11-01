#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <unordered_map>
#include <chrono>
#include "workbench/core/result.hpp"
#include "workbench/core/json.hpp"

namespace enginex::core {

enum class JobStatus {
    Queued = 0,
    Pending = 0, // Backward-compatible alias
    Running,
    Completed,
    Cancelled,
    Failed
};

std::string job_status_to_string(JobStatus status);

struct JobInfo {
    std::string id;
    std::string type;
    JobStatus status{JobStatus::Queued};
    float progress{0.0f}; // 0.0 to 1.0
    std::string status_message;
    std::string result_json;
    std::string error_message;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point started_at;
    bool has_started{false};
    std::chrono::system_clock::time_point completed_at;
    bool has_completed{false};
    std::atomic<bool> cancel_requested{false};

    nlohmann::json to_json() const;
};

class JobManager {
public:
    static JobManager& instance();

    // Start / configure bounded worker pool
    void init_worker_pool(size_t num_threads = 2, size_t max_queue_size = 64);
    void stop_worker_pool();

    // Create a new tracked job without worker scheduling
    std::string create_job(const std::string& type, const std::string& initial_message = "Job initialized");

    // Asynchronously submit a job to the bounded worker pool
    Result<std::string> submit_job(
        const std::string& type,
        std::function<void(const std::string& job_id)> task,
        const std::string& initial_message = "Job queued"
    );

    // Check if cancellation has been requested for a job
    bool is_cancel_requested(const std::string& job_id);

    // Request cooperative cancellation of a job
    Result<void> request_cancel(const std::string& job_id);

    // Update progress and status
    void update_progress(const std::string& job_id, float progress, const std::string& message = "");

    // Set terminal state: completed (stores result before state transition)
    void mark_completed(const std::string& job_id, const std::string& result_json, const std::string& final_message = "Complete");

    // Set terminal state: failed
    void mark_failed(const std::string& job_id, const std::string& error_message);

    // Set terminal state: cancelled
    void mark_cancelled(const std::string& job_id, const std::string& message = "Job was cancelled by user");

    // Get snapshot of a job's status
    Result<nlohmann::json> get_job(const std::string& job_id);

    // Get result of a completed job (returns error if not ready, cancelled, or failed)
    Result<nlohmann::json> get_job_result(const std::string& job_id);

    // Clear completed/cancelled/failed jobs older than 1 hour
    void clear_old_jobs();

    // Reset/clear all jobs (used for clean testing)
    void reset();

    // Diagnostics
    size_t queue_size() const;
    size_t active_workers() const;
    bool is_pool_stopped() const;

private:
    JobManager();
    ~JobManager();
    JobManager(const JobManager&) = delete;
    JobManager& operator=(const JobManager&) = delete;

    void worker_loop();

    std::mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<JobInfo>> m_jobs;
    std::atomic<uint64_t> m_next_id{1000};

    // Bounded worker pool state
    mutable std::mutex m_pool_mutex;
    std::condition_variable m_pool_cv;
    std::vector<std::thread> m_worker_threads;
    std::queue<std::pair<std::string, std::function<void(const std::string&)>>> m_task_queue;
    size_t m_max_queue_size{64};
    std::atomic<bool> m_pool_stopped{false};
    std::atomic<size_t> m_running_tasks{0};
};

} // namespace enginex::core

