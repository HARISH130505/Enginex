#include "workbench/core/job_manager.hpp"
#include <iomanip>
#include <sstream>

namespace enginex::core {

std::string job_status_to_string(JobStatus status) {
    switch (status) {
        case JobStatus::Pending: return "PENDING";
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

    if (!result_json.empty()) {
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
    if (status == JobStatus::Completed || status == JobStatus::Cancelled || status == JobStatus::Failed) {
        j["completed_at"] = to_iso(completed_at);
    }

    return j;
}

JobManager& JobManager::instance() {
    static JobManager s_instance;
    return s_instance;
}

std::string JobManager::create_job(const std::string& type, const std::string& initial_message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    uint64_t id_num = m_next_id.fetch_add(1);
    std::string id = "job_" + std::to_string(id_num);

    auto job = std::make_shared<JobInfo>();
    job->id = id;
    job->type = type;
    job->status = JobStatus::Pending;
    job->progress = 0.0f;
    job->status_message = initial_message;
    job->created_at = std::chrono::system_clock::now();

    m_jobs[id] = job;
    return id;
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
    job->status_message = "Cancellation requested by user";
    return Result<void>::ok();
}

void JobManager::update_progress(const std::string& job_id, float progress, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        auto& job = it->second;
        if (job->status == JobStatus::Pending) {
            job->status = JobStatus::Running;
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
        job->status = JobStatus::Completed;
        job->progress = 1.0f;
        job->result_json = result_json;
        job->status_message = final_message;
        job->completed_at = std::chrono::system_clock::now();
    }
}

void JobManager::mark_failed(const std::string& job_id, const std::string& error_message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        auto& job = it->second;
        job->status = JobStatus::Failed;
        job->error_message = error_message;
        job->status_message = "Failed: " + error_message;
        job->completed_at = std::chrono::system_clock::now();
    }
}

void JobManager::mark_cancelled(const std::string& job_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_jobs.find(job_id);
    if (it != m_jobs.end()) {
        auto& job = it->second;
        job->status = JobStatus::Cancelled;
        job->status_message = message;
        job->completed_at = std::chrono::system_clock::now();
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

void JobManager::clear_old_jobs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::system_clock::now();
    for (auto it = m_jobs.begin(); it != m_jobs.end(); ) {
        if (it->second->status == JobStatus::Completed ||
            it->second->status == JobStatus::Cancelled ||
            it->second->status == JobStatus::Failed) {
            auto elapsed = std::chrono::duration_cast<std::chrono::hours>(now - it->second->completed_at).count();
            if (elapsed >= 1) {
                it = m_jobs.erase(it);
                continue;
            }
        }
        ++it;
    }
}

} // namespace enginex::core

