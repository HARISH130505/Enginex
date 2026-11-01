#include "workbench/core/job_manager.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace enginex::core {

std::string job_status_to_string(JobStatus status) {
    switch (status) {
        case JobStatus::Queued: return "QUEUED";
        case JobStatus::Running: return "RUNNING";
        case JobStatus::Completed: return "COMPLETED";
        case JobStatus::Cancelled: return "CANCELLED";
        case JobStatus::Failed: return "FAILED";
        default: return "UNKNOWN";
    }
}

nlohmann::json JobInfo::to_json() const {
    nlohmann::json j;
    j["id"] = id;
    j["type"] = type;
    j["status"] = job_status_to_string(status);
    j["progress"] = progress;
    j["status_message"] = status_message;
    j["cancel_requested"] = cancel_requested.load();

    if (!error_message.empty()) {
        j["error_message"] = error_message;
    }

    if (!result_json.empty() && status == JobStatus::Completed) {
        try {
            j["result"] = nlohmann::json::parse(result_json);
        } catch (...) {
            j["result"] = result_json;
        }
    } else {
        j["result"] = nullptr;
    }

    auto to_iso = [](const std::chrono::system_clock::time_point& tp) {
        auto time_t_val = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    };

    j["created_at"] = to_iso(created_at);

    if (has_started) {
        j["started_at"] = to_iso(started_at);
    }

    if (has_completed) {
        j["completed_at"] = to_iso(completed_at);
    }

    return j;
}

JobManager& JobManager::instance() {
    static JobManager s_instance;
    return s_instance;
}

JobManager::JobManager() {
    init_worker_pool(2, 64);
}

JobManager::~JobManager() {
    stop_worker_pool();
}

void JobManager::init_worker_pool(size_t num_threads, size_t max_queue_size) {
    std::unique_lock<std::mutex> pool_lock(m_pool_mutex);
    if (!m_worker_threads.empty() && !m_pool_stopped.load()) {
        return; // Already initialized and running
    }

    m_pool_stopped.store(false);
    m_max_queue_size = max_queue_size;

    m_worker_threads.clear();
    m_worker_threads.reserve(num_threads);
    for (size_t i = 0; i < num_threads; ++i) {
        m_worker_threads.emplace_back(&JobManager::worker_loop, this);
    }
}

void JobManager::stop_worker_pool() {
    {
        std::unique_lock<std::mutex> pool_lock(m_pool_mutex);
        if (m_pool_stopped.load()) {
            return;
        }
        m_pool_stopped.store(true);
    }
    m_pool_cv.notify_all();

    for (auto& th : m_worker_threads) {
        if (th.joinable()) {
            th.join();
        }
    }
    m_worker_threads.clear();
}

void JobManager::worker_loop() {
    while (true) {
        std::pair<std::string, std::function<void(const std::string&)>> item;
        {
            std::unique_lock<std::mutex> lock(m_pool_mutex);
            m_pool_cv.wait(lock, [this]() {
                return m_pool_stopped.load() || !m_task_queue.empty();
            });

            if (m_pool_stopped.load() && m_task_queue.empty()) {
                break;
            }

            item = std::move(m_task_queue.front());
            m_task_queue.pop();
        }

        const std::string& job_id = item.first;
        auto& task = item.second;

        bool skip_task = false;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto it = m_jobs.find(job_id);
            if (it != m_jobs.end()) {
                auto& job = it->second;
                if (job->cancel_requested.load() || job->status == JobStatus::Cancelled) {
                    job->status = JobStatus::Cancelled;
                    job->status_message = "Cancelled before execution";
                    job->completed_at = std::chrono::system_clock::now();
                    job->has_completed = true;
                    skip_task = true;
                } else {
                    job->status = JobStatus::Running;
                    job->started_at = std::chrono::system_clock::now();
                    job->has_started = true;
                    job->status_message = "Running";
                }
            } else {
                skip_task = true;
            }
        }

        if (skip_task) {
            continue;
        }

        m_running_tasks.fetch_add(1);
        try {
            task(job_id);
        } catch (const std::exception& e) {
            mark_failed(job_id, e.what());
        } catch (...) {
            mark_failed(job_id, "Unknown exception during job execution");
        }
        m_running_tasks.fetch_sub(1);
    }
}

std::string JobManager::create_job(const std::string& type, const std::string& initial_message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t id_num = m_next_id.fetch_add(1);
    std::string id = "job_" + std::to_string(id_num);

    auto job = std::make_shared<JobInfo>();
    job->id = id;
    job->type = type;
    job->status = JobStatus::Queued;
    job->progress = 0.0f;
    job->status_message = initial_message;
    job->created_at = std::chrono::system_clock::now();
    job->has_started = false;
    job->has_completed = false;

    m_jobs[id] = job;
    return id;
}

Result<std::string> JobManager::submit_job(
    const std::string& type,
    std::function<void(const std::string& job_id)> task,
    const std::string& initial_message)
{
    if (m_pool_stopped.load()) {
        return Result<std::string>::fail(StatusCode::Unavailable, "Job worker pool is stopped");
    }

    std::string job_id;
    {
        std::unique_lock<std::mutex> pool_lock(m_pool_mutex);
        if (m_task_queue.size() >= m_max_queue_size) {
            return Result<std::string>::fail(StatusCode::ResourceExhausted, "Job queue is full");
        }

        job_id = create_job(type, initial_message);
        m_task_queue.emplace(job_id, std::move(task));
    }

    m_pool_cv.notify_one();
    return Result<std::string>::ok(job_id);
}

bool JobManager::is_cancel_requested(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        return it->second->cancel_requested.load();
    }
    return false;
}

Result<void> JobManager::request_cancel(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it == m_jobs.end()) {
        return Result<void>::fail(StatusCode::NotFound, "Job ID not found: " + job_id);
    }

    auto& job = it->second;
    if (job->status == JobStatus::Completed || job->status == JobStatus::Cancelled || job->status == JobStatus::Failed) {
        return Result<void>::fail(StatusCode::InvalidArgument, "Job is already in terminal state: " + job_status_to_string(job->status));
    }

    job->cancel_requested.store(true);
    if (job->status == JobStatus::Queued) {
        job->status = JobStatus::Cancelled;
        job->status_message = "Cancelled while queued";
        job->completed_at = std::chrono::system_clock::now();
        job->has_completed = true;
    } else {
        job->status_message = "Cancellation requested by user";
    }

    return Result<void>::ok();
}

void JobManager::update_progress(const std::string& job_id, float progress, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        auto& job = it->second;
        if (job->status == JobStatus::Queued) {
            job->status = JobStatus::Running;
            if (!job->has_started) {
                job->started_at = std::chrono::system_clock::now();
                job->has_started = true;
            }
        }
        if (job->status == JobStatus::Running) {
            job->progress = std::max(0.0f, std::min(1.0f, progress));
            if (!message.empty()) {
                job->status_message = message;
            }
        }
    }
}

void JobManager::mark_completed(const std::string& job_id, const std::string& result_json, const std::string& final_message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        auto& job = it->second;
        // If cancellation requested or already cancelled, do NOT mark completed
        if (job->cancel_requested.load() || job->status == JobStatus::Cancelled) {
            job->status = JobStatus::Cancelled;
            job->result_json.clear();
            job->completed_at = std::chrono::system_clock::now();
            job->has_completed = true;
            return;
        }
        if (job->status == JobStatus::Failed) {
            return;
        }

        // Store result successfully BEFORE updating status to Completed
        job->result_json = result_json;
        job->progress = 1.0f;
        job->status_message = final_message;
        job->completed_at = std::chrono::system_clock::now();
        job->has_completed = true;
        job->status = JobStatus::Completed;
    }
}

void JobManager::mark_failed(const std::string& job_id, const std::string& error_message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        auto& job = it->second;
        if (job->status == JobStatus::Completed || job->status == JobStatus::Cancelled) {
            return;
        }
        job->status = JobStatus::Failed;
        job->result_json.clear();
        job->error_message = error_message;
        job->status_message = "Failed: " + error_message;
        job->completed_at = std::chrono::system_clock::now();
        job->has_completed = true;
    }
}

void JobManager::mark_cancelled(const std::string& job_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        auto& job = it->second;
        if (job->status == JobStatus::Completed) {
            return; // Cannot cancel completed job
        }
        job->status = JobStatus::Cancelled;
        job->result_json.clear();
        job->status_message = message;
        job->completed_at = std::chrono::system_clock::now();
        job->has_completed = true;
    }
}

Result<nlohmann::json> JobManager::get_job(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it == m_jobs.end()) {
        return Result<nlohmann::json>::fail(StatusCode::NotFound, "Job ID not found: " + job_id);
    }
    return Result<nlohmann::json>::ok(it->second->to_json());
}

Result<nlohmann::json> JobManager::get_job_result(const std::string& job_id) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it == m_jobs.end()) {
        return Result<nlohmann::json>::fail(StatusCode::NotFound, "Job ID not found: " + job_id);
    }

    const auto& job = it->second;
    if (job->status == JobStatus::Queued || job->status == JobStatus::Running) {
        return Result<nlohmann::json>::fail(StatusCode::Unavailable, "Job is still in progress (status: " + job_status_to_string(job->status) + ")");
    }

    if (job->status == JobStatus::Cancelled) {
        return Result<nlohmann::json>::fail(StatusCode::Cancelled, "Job was cancelled");
    }

    if (job->status == JobStatus::Failed) {
        return Result<nlohmann::json>::fail(StatusCode::InternalError, "Job failed: " + job->error_message);
    }

    if (job->result_json.empty()) {
        return Result<nlohmann::json>::ok(nlohmann::json::object());
    }

    try {
        return Result<nlohmann::json>::ok(nlohmann::json::parse(job->result_json));
    } catch (...) {
        return Result<nlohmann::json>::ok(job->result_json);
    }
}

void JobManager::clear_old_jobs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    for (auto it = m_jobs.begin(); it != m_jobs.end(); ) {
        if (it->second->has_completed) {
            auto elapsed = std::chrono::duration_cast<std::chrono::hours>(now - it->second->completed_at).count();
            if (elapsed >= 1) {
                it = m_jobs.erase(it);
                continue;
            }
        }
        ++it;
    }
}

void JobManager::reset() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_jobs.clear();
}

size_t JobManager::queue_size() const {
    std::unique_lock<std::mutex> lock(m_pool_mutex);
    return m_task_queue.size();
}

size_t JobManager::active_workers() const {
    return m_running_tasks.load();
}

bool JobManager::is_pool_stopped() const {
    return m_pool_stopped.load();
}

} // namespace enginex::core
