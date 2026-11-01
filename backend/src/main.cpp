#include "workbench/api/http_server.hpp"
#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>

static enginex::api::HttpServer* g_server = nullptr;

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n[ENGINEX Backend] Received shutdown signal. Gracefully stopping server...\n";
        if (g_server) {
            g_server->stop();
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=================================================================\n";
    std::cout << "  ENGINEX Workbench — Local Engineering Service (C++17)\n";
    std::cout << "  PASS Science Works R&D Internship 2026–2027 Evaluation\n";
    std::cout << "=================================================================\n";

    enginex::api::ServerConfig config;
    config.host = "127.0.0.1";
    config.port = 8080;
    config.web_root = "";

    // Parse CLI arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            config.port = static_cast<uint16_t>(std::stoi(argv[++i]));
        } else if (arg == "--web-root" && i + 1 < argc) {
            config.web_root = argv[++i];
        } else if (arg == "--host" && i + 1 < argc) {
            config.host = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: backend_server.exe [OPTIONS]\n"
                      << "Options:\n"
                      << "  --port <port>       Port to bind (default: 8080)\n"
                      << "  --host <ip>         Host address to bind (default: 127.0.0.1)\n"
                      << "  --web-root <dir>    Path to exported Next.js static files\n"
                      << "  --help, -h          Show this help message\n";
            return 0;
        }
    }

    enginex::api::HttpServer server(config);
    g_server = &server;

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "[ENGINEX Backend] Initializing server on " << config.host << ":" << config.port << "...\n";
    if (!server.start()) {
        std::cerr << "[ENGINEX Backend] Error: Failed to bind or start HTTP server on port " << config.port << "\n";
        return 1;
    }

    std::cout << "[ENGINEX Backend] Server stopped cleanly.\n";
    return 0;
}

