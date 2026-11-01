#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "workbench/api/http_server.hpp"
#include "workbench/api/httplib.h"
#include "workbench/core/json.hpp"
#include "workbench/core/job_manager.hpp"
#include "workbench/geometry/stl_parser.hpp"
#include "workbench/geometry/csv_parser.hpp"
#include "workbench/thermal/solver_1d.hpp"
#include "workbench/performance/benchmark_harness.hpp"
#include "workbench/hardware/device_adapter.hpp"
#include "workbench/hardware/mock_adapters.hpp"
#include "workbench/neural/backprop.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <memory>

namespace enginex::api {

// Singleton registry of all hardware adapters (initialized on first use)
static std::vector<std::unique_ptr<hardware::IDeviceAdapter>>& get_devices() {
    static std::vector<std::unique_ptr<hardware::IDeviceAdapter>> devices;
    static bool initialized = false;
    if (!initialized) {
        devices.push_back(std::make_unique<hardware::MockMCUAdapter>());
        devices.push_back(std::make_unique<hardware::MockFPGAAdapter>());
        devices.push_back(std::make_unique<hardware::MockJTAGAdapter>());
        initialized = true;
    }
    return devices;
}

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

    // 6. Thermal FEA solver endpoint
    m_server->Post("/api/v1/thermal/solve", [](const httplib::Request& req, httplib::Response& res) {
        thermal::RodParameters params;
        std::vector<size_t> mesh_sizes = {10, 20, 40, 80};

        try {
            if (!req.body.empty()) {
                auto body_json = nlohmann::json::parse(req.body);
                if (body_json.contains("L")) params.L = body_json["L"].get<double>();
                if (body_json.contains("A")) params.A = body_json["A"].get<double>();
                if (body_json.contains("k")) params.k = body_json["k"].get<double>();
                if (body_json.contains("q")) params.q = body_json["q"].get<double>();
                if (body_json.contains("T_left")) params.T_left = body_json["T_left"].get<double>();
                if (body_json.contains("T_right")) params.T_right = body_json["T_right"].get<double>();
                if (body_json.contains("mesh_sizes") && body_json["mesh_sizes"].is_array()) {
                    mesh_sizes.clear();
                    for (const auto& n : body_json["mesh_sizes"]) {
                        mesh_sizes.push_back(n.get<size_t>());
                    }
                }
            }
        } catch (const std::exception& e) {
            nlohmann::json err = {
                {"error", std::string("Invalid request body: ") + e.what()},
                {"code", "INVALID_INPUT"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
            return;
        }

        auto result = thermal::Solver1D::mesh_study(params, mesh_sizes);
        if (result.is_ok()) {
            res.set_content(result.value().to_json().dump(), "application/json");
            res.status = 200;
        } else {
            nlohmann::json err = {
                {"error", result.error().message},
                {"code", "THERMAL_SOLVER_ERROR"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
        }
    });

    // 7. Multicore benchmark execution endpoint
    m_server->Post("/api/v1/performance/run", [](const httplib::Request& req, httplib::Response& res) {
        performance::BenchmarkConfig config;

        try {
            if (!req.body.empty()) {
                auto body_json = nlohmann::json::parse(req.body);
                config = performance::BenchmarkConfig::from_json(body_json);
            }
        } catch (const std::exception& e) {
            nlohmann::json err = {
                {"error", std::string("Invalid benchmark configuration: ") + e.what()},
                {"code", "INVALID_INPUT"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
            return;
        }

        auto result = performance::BenchmarkHarness::run_benchmark(config);
        if (result.is_ok()) {
            res.set_content(result.value().to_json().dump(), "application/json");
            res.status = 200;
        } else {
            nlohmann::json err = {
                {"error", result.error().message},
                {"code", "BENCHMARK_ERROR"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
        }
    });

    // 8. Hardware: list all devices
    m_server->Get("/api/v1/hardware/devices", [](const httplib::Request&, httplib::Response& res) {
        auto& devs = get_devices();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& d : devs) {
            arr.push_back(d->describe());
        }
        nlohmann::json resp = {{"devices", arr}, {"count", arr.size()}};
        res.set_content(resp.dump(2), "application/json");
        res.status = 200;
    });

    // 9. Hardware: connect device by id
    m_server->Post(R"(/api/v1/hardware/([^/]+)/connect)", [](const httplib::Request& req, httplib::Response& res) {
        std::string dev_id = req.matches[1];
        auto& devs = get_devices();
        for (auto& d : devs) {
            if (d->id() == dev_id) {
                auto r = d->connect();
                if (r.is_ok()) {
                    nlohmann::json ok = {{"status", "connected"}, {"device_id", dev_id}};
                    res.set_content(ok.dump(2), "application/json");
                    res.status = 200;
                } else {
                    nlohmann::json err = {{"error", r.error().message}, {"code", "CONNECT_ERROR"}};
                    res.set_content(err.dump(2), "application/json");
                    res.status = 400;
                }
                return;
            }
        }
        nlohmann::json err = {{"error", "Device not found: " + dev_id}, {"code", "NOT_FOUND"}};
        res.set_content(err.dump(2), "application/json");
        res.status = 404;
    });

    // 10. Hardware: disconnect device by id
    m_server->Post(R"(/api/v1/hardware/([^/]+)/disconnect)", [](const httplib::Request& req, httplib::Response& res) {
        std::string dev_id = req.matches[1];
        auto& devs = get_devices();
        for (auto& d : devs) {
            if (d->id() == dev_id) {
                auto r = d->disconnect();
                nlohmann::json ok = {{"status", "disconnected"}, {"device_id", dev_id}};
                res.set_content(ok.dump(2), "application/json");
                res.status = 200;
                return;
            }
        }
        nlohmann::json err = {{"error", "Device not found: " + dev_id}, {"code", "NOT_FOUND"}};
        res.set_content(err.dump(2), "application/json");
        res.status = 404;
    });

    // 11. Hardware: read telemetry from device (auto-recover on fault)
    m_server->Get(R"(/api/v1/hardware/([^/]+)/telemetry)", [](const httplib::Request& req, httplib::Response& res) {
        std::string dev_id = req.matches[1];
        auto& devs = get_devices();
        for (auto& d : devs) {
            if (d->id() == dev_id) {
                // If faulted, auto-recover before reading
                if (d->state() == hardware::ConnectionState::Fault) {
                    d->recover();
                }
                auto t = d->read_telemetry();
                if (t.is_ok()) {
                    res.set_content(t.value().to_json().dump(2), "application/json");
                    res.status = 200;
                } else {
                    nlohmann::json err = {
                        {"error", t.error().message},
                        {"code", "TELEMETRY_ERROR"},
                        {"device_state", hardware::connection_state_to_string(d->state())}
                    };
                    res.set_content(err.dump(2), "application/json");
                    res.status = 503;
                }
                return;
            }
        }
        nlohmann::json err = {{"error", "Device not found: " + dev_id}, {"code", "NOT_FOUND"}};
        res.set_content(err.dump(2), "application/json");
        res.status = 404;
    });

    // 12. Neural backpropagation training & gradient verification endpoint
    m_server->Post("/api/v1/neural/train", [](const httplib::Request& req, httplib::Response& res) {
        double x = 1.0;
        double y = 2.0;
        neural::ModelParameters params;
        double lr = 0.1;
        size_t steps = 100;
        double eps = 1.0e-6;

        try {
            if (!req.body.empty()) {
                auto b = nlohmann::json::parse(req.body);
                if (b.contains("x")) x = b["x"].get<double>();
                if (b.contains("y")) y = b["y"].get<double>();
                if (b.contains("w")) params.w = b["w"].get<double>();
                if (b.contains("b")) params.b = b["b"].get<double>();
                if (b.contains("v")) params.v = b["v"].get<double>();
                if (b.contains("c")) params.c = b["c"].get<double>();
                if (b.contains("lr")) lr = b["lr"].get<double>();
                if (b.contains("steps")) steps = b["steps"].get<size_t>();
                if (b.contains("epsilon")) eps = b["epsilon"].get<double>();
            }
        } catch (const std::exception& e) {
            nlohmann::json err = {
                {"error", std::string("Invalid input parameters: ") + e.what()},
                {"code", "INVALID_INPUT"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
            return;
        }

        auto result = neural::BackpropCore::train(x, y, params, lr, steps, eps);
        if (result.is_ok()) {
            res.set_content(result.value().to_json().dump(), "application/json");
            res.status = 200;
        } else {
            nlohmann::json err = {
                {"error", result.error().message},
                {"code", "TRAIN_ERROR"}
            };
            res.set_content(err.dump(2), "application/json");
            res.status = 400;
        }
    });

    // 13. Static web assets mounting (for exported Next.js client)
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

