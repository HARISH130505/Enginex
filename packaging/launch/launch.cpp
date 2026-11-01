#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shellapi.h>
#include <wininet.h>

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <filesystem>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "wininet.lib")

namespace fs = std::filesystem;

// Acquire a guaranteed free ephemeral port on loopback 127.0.0.1
uint16_t find_free_port() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 8080; // fallback
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 8080;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr("127.0.0.1");
    service.sin_port = 0; // Kernel selects free ephemeral port

    if (bind(sock, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return 8080;
    }

    int len = sizeof(service);
    if (getsockname(sock, (SOCKADDR*)&service, &len) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return 8080;
    }

    uint16_t port = ntohs(service.sin_port);
    closesocket(sock);
    WSACleanup();
    return port;
}

// Simple HTTP GET check using Winsock to avoid external dependencies
bool check_health(uint16_t port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(port);

    // Set non-blocking with 500ms timeout
    DWORD timeout = 500;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    if (connect(sock, (SOCKADDR*)&server_addr, sizeof(server_addr)) != 0) {
        closesocket(sock);
        WSACleanup();
        return false;
    }

    std::string req = "GET /api/v1/health HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
    send(sock, req.c_str(), static_cast<int>(req.length()), 0);

    char buffer[1024] = {0};
    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
    closesocket(sock);
    WSACleanup();

    if (bytes > 0) {
        std::string resp(buffer, bytes);
        return (resp.find("200 OK") != std::string::npos);
    }
    return false;
}

int main() {
    std::wcout << L"=================================================================\n";
    std::wcout << L"  ENGINEX Workbench — Desktop Launcher (Windows 11 x64)\n";
    std::wcout << L"  PASS Science Works R&D Internship 2026–2027 Evaluation\n";
    std::wcout << L"=================================================================\n";

    // 1. Locate directory containing Launch.exe
    wchar_t exe_path_buf[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path_buf, MAX_PATH);
    fs::path exe_dir = fs::path(exe_path_buf).parent_path();

    // Determine paths to backend executable and web directory
    fs::path backend_exe = exe_dir / "bin" / "backend_server.exe";
    if (!fs::exists(backend_exe)) {
        // Fallback for build tree structure
        backend_exe = exe_dir / ".." / "bin" / "backend_server.exe";
        if (!fs::exists(backend_exe)) {
            backend_exe = exe_dir / "backend_server.exe";
        }
    }

    fs::path web_dir = exe_dir / "web";
    if (!fs::exists(web_dir)) {
        web_dir = exe_dir / ".." / "frontend" / "out";
        if (!fs::exists(web_dir)) {
            web_dir = exe_dir / "frontend" / "out";
        }
    }

    if (!fs::exists(backend_exe)) {
        std::wcerr << L"[ERROR] backend_server.exe not found at: " << backend_exe.wstring() << L"\n";
        std::wcout << L"Press ENTER to exit...";
        std::cin.get();
        return 1;
    }

    // 2. Select ephemeral port
    uint16_t port = find_free_port();
    std::wcout << L"[INFO] Bound ephemeral loopback port: " << port << L"\n";

    // 3. Create Windows Job Object to guarantee clean child process termination
    HANDLE hJob = CreateJobObjectW(NULL, NULL);
    if (hJob) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {0};
        jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
        SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli));
    }

    // 4. Spawn backend server
    std::wstring cmd = L"\"" + backend_exe.wstring() + L"\" --port " + std::to_wstring(port);
    if (fs::exists(web_dir)) {
        cmd += L" --web-root \"" + web_dir.wstring() + L"\"";
    }

    STARTUPINFOW si = {sizeof(si)};
    PROCESS_INFORMATION pi = {0};

    std::wcout << L"[INFO] Starting backend server: " << cmd << L"\n";
    std::vector<wchar_t> cmd_buf(cmd.begin(), cmd.end());
    cmd_buf.push_back(0);

    if (!CreateProcessW(NULL, cmd_buf.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::wcerr << L"[ERROR] Failed to start backend server. GetLastError = " << GetLastError() << L"\n";
        return 1;
    }

    if (hJob) {
        AssignProcessToJobObject(hJob, pi.hProcess);
    }

    // 5. Poll health check
    std::wcout << L"[INFO] Waiting for backend readiness at http://127.0.0.1:" << port << L"/api/v1/health...\n";
    bool ready = false;
    for (int i = 0; i < 50; ++i) { // 50 * 200ms = 10s timeout
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (check_health(port)) {
            ready = true;
            break;
        }
    }

    if (!ready) {
        std::wcerr << L"[ERROR] Timed out waiting for backend server to become ready.\n";
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (hJob) CloseHandle(hJob);
        return 1;
    }

    std::wcout << L"[SUCCESS] Backend ready! Launching web interface...\n";
    std::wstring url = L"http://127.0.0.1:" + std::to_wstring(port) + L"/";
    ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);

    std::wcout << L"\n=================================================================\n";
    std::wcout << L"  ENGINEX Workbench is running.\n";
    std::wcout << L"  URL: " << url << L"\n";
    std::wcout << L"  Press ENTER or Ctrl+C in this console to shut down workbench.\n";
    std::wcout << L"=================================================================\n";

    std::cin.get();

    std::wcout << L"[INFO] Shutting down ENGINEX Workbench...\n";
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    if (hJob) CloseHandle(hJob);

    return 0;
}

