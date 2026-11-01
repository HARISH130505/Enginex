#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "workbench/api/http_server.hpp"
#include "workbench/api/httplib.h"
#include "workbench/core/json.hpp"
#include "workbench/core/job_manager.hpp"
#include "workbench/geometry/stl_parser.hpp"
#include "workbench/geometry/csv_parser.hpp"

#include <iostream>
#include <filesystem>
#include <chrono>

namespace enginex::api {

HttpServer::HttpServer(ServerConfig config)
    : m_config(std::move(config)),
      m_server(std::make_unique<httplib::Server>()),
      m_is_running(false),
      m_actual_port(m_config.port) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::setup_routes() {
    // Enable CORS for development convenience (though production runs on single origin)
    m_server->set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });

    // Handle OPTIONS pre-flight
    m_server->Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // 1. Health check endpoint
    m_server->Get("/api/v1/health", [this](const httplib::Request&, httplib::Response& res) {
        static auto start_time = std::chrono::steady_clock::now();
        auto uptime_sec = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - start_time).count();

        nlohmann::json response = {
            {"status", "ok"},
            {"application", "ENGINEX Workbench"},
            {"project", "ENGINEX — Unified Engineering Simulation & Analysis Platform"},
            {"version", "1.0.0"},
            {"backend", "C++17"},
            {"platform", "Windows 11 x64"},
            {"uptime_seconds", uptime_sec},
            {"bound_port", m_actual_port},
            {"routes", {
                "/geometry",
                "/thermal",
                "/performance",
                "/hardware",
                "/backpropagation"
            }}
        };

        res.set_content(response.dump(2), "application/json");
        res.status = 200;
    });

    // 2. Job polling endpoint
    m_server->Get(R"(/api/v1/jobs/([^/]+))", [](const httplib::Request& req, httplib::Response& res) {
        std::string job_id = req.matches[1];
        auto result = core::JobManager::instance().get_job(job_id);
        if (result.is_ok()) {
            res.set_content(result.value().dump(2), "application/json");
            res.status = 200;
        } else {
            nlohmann::json err = {
                {"error", result.error().message},
                {"code", "NOT_FOUND"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 404;
        }
    });

    // 3. Job cancellation endpoint
    m_server->Post(R"(/api/v1/jobs/([^/]+)/cancel)", [](const httplib::Request& req, httplib::Response& res) {
        std::string job_id = req.matches[1];
        auto result = core::JobManager::instance().request_cancel(job_id);
        if (result.is_ok()) {
            nlohmann::json ok = {
                {"status", "cancellation_requested"},
                {"job_id", job_id}
            };
            res.set_content(ok.dump(2), "application/json");
            res.status = 200;
        } else {
            nlohmann::json err = {
                {"error", result.error().message},
                {"code", "INVALID_ACTION"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
        }
    });

    // 4. Geometry STL inspection endpoint
    m_server->Post("/api/v1/geometry/stl/inspect", [](const httplib::Request& req, httplib::Response& res) {
        std::string content;
        std::string filename = "uploaded.stl";

        if (req.is_multipart_form_data()) {
            if (req.has_file("file")) {
                const auto& file = req.get_file_value("file");
                content = file.content;
                filename = file.filename;
            } else {
                nlohmann::json err = {{"error", "Multipart upload missing 'file' field"}, {"code", "INVALID_INPUT"}};
                res.set_content(err.dump(2), "application/json");
                res.status = 400;
                return;
            }
        } else {
            bool parsed_json = false;
            try {
                auto body_json = nlohmann::json::parse(req.body);
                if (body_json.is_object() && body_json.contains("content")) {
                    if (body_json["content"].is_string()) {
                        content = body_json["content"].get<std::string>();
                        parsed_json = true;
                    } else if (body_json["content"].is_object() && body_json["content"].contains("value")) {
                        content = body_json["content"]["value"].get<std::string>();
                        parsed_json = true;
                    }
                    filename = body_json.value("filename", "geometry.stl");
                }
            } catch (...) {
                // Not JSON, treat as raw content
            }
            if (!parsed_json) {
                content = req.body;
            }
        }

        auto result = geometry::StlParser::parse_ascii_string(content, filename);
        if (result.is_ok()) {
            res.set_content(result.value().to_json(true).dump(), "application/json");
            res.status = 200;
        } else {
            nlohmann::json err = {
                {"error", result.error().message},
                {"code", "STL_PARSE_ERROR"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
        }
    });

    // 5. Geometry ANSYS Nodal Temperature CSV import endpoint
    m_server->Post("/api/v1/geometry/temperature/import", [](const httplib::Request& req, httplib::Response& res) {
        std::string content;
        std::string filename = "nodal_temperature.csv";

        if (req.is_multipart_form_data()) {
            if (req.has_file("file")) {
                const auto& file = req.get_file_value("file");
                content = file.content;
                filename = file.filename;
            } else {
                nlohmann::json err = {{"error", "Multipart upload missing 'file' field"}, {"code", "INVALID_INPUT"}};
                res.set_content(err.dump(2), "application/json");
                res.status = 400;
                return;
            }
        } else {
            bool parsed_json = false;
            try {
                auto body_json = nlohmann::json::parse(req.body);
                if (body_json.is_object() && body_json.contains("content")) {
                    if (body_json["content"].is_string()) {
                        content = body_json["content"].get<std::string>();
                        parsed_json = true;
                    } else if (body_json["content"].is_object() && body_json["content"].contains("value")) {
                        content = body_json["content"]["value"].get<std::string>();
                        parsed_json = true;
                    }
                    filename = body_json.value("filename", "nodal_temperature.csv");
                }
            } catch (...) {
                // Not JSON, treat as raw content
            }
            if (!parsed_json) {
                content = req.body;
            }
        }

        auto result = geometry::CsvParser::parse_ansys_csv_string(content, filename);
        if (result.is_ok()) {
            res.set_content(result.value().to_json(true).dump(), "application/json");
            res.status = 200;
        } else {
            nlohmann::json err = {
                {"error", result.error().message},
                {"code", "CSV_PARSE_ERROR"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
        }
    });

    // 6. Static web assets mounting (for exported Next.js client)
    if (!m_config.web_root.empty()) {
        std::error_code ec;
        if (std::filesystem::exists(m_config.web_root, ec)) {
            std::cout << "[ENGINEX Backend] Mounting static frontend directory: " << m_config.web_root << "\n";
            m_server->set_mount_point("/", m_config.web_root);
        } else {
            std::cerr << "[ENGINEX Backend] Warning: Static web root not found: " << m_config.web_root << "\n";
        }
    }
}

bool HttpServer::start() {
    setup_routes();
    m_is_running = true;
    m_actual_port = m_config.port;

    std::cout << "[ENGINEX Backend] Listening on http://" << m_config.host << ":" << m_actual_port << "\n";
    bool success = m_server->listen(m_config.host, m_actual_port);
    m_is_running = false;
    return success;
}

void HttpServer::stop() {
    if (m_is_running && m_server) {
        m_server->stop();
        m_is_running = false;
    }
}

bool HttpServer::is_running() const noexcept {
    return m_is_running;
}

uint16_t HttpServer::get_bound_port() const noexcept {
    return m_actual_port;
}

} // namespace enginex::api

