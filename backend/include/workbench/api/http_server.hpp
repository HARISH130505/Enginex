#pragma once

#include <string>
#include <memory>
#include <cstdint>
#include <functional>

namespace httplib {
    class Server;
}

namespace enginex::api {

struct ServerConfig {
    std::string host = "127.0.0.1";
    uint16_t port = 8080;
    std::string web_root = ""; // Directory containing static Next.js export
};

class HttpServer {
public:
    explicit HttpServer(ServerConfig config);
    ~HttpServer();

    // Initialize routes and static file handlers
    void setup_routes();

    // Start server listening (blocks until stop() is called)
    bool start();

    // Stop server gracefully
    void stop();

    // Check if server is running
    bool is_running() const noexcept;

    // Get the actual bound port (useful when port 0 is passed for ephemeral port)
    uint16_t get_bound_port() const noexcept;

private:
    ServerConfig m_config;
    std::unique_ptr<httplib::Server> m_server;
    bool m_is_running{false};
    uint16_t m_actual_port{0};
};

} // namespace enginex::api

