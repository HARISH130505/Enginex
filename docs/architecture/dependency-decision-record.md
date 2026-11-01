# Dependency Decision Record (DDR)
## ENGINEX — Unified Engineering Simulation & Analysis Platform (ENGINEX Workbench)

| Decision ID | Area | Selected Technology / Library | Version | License | Justification & Architectural Trade-offs | Fallback Strategy |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **DDR-01** | C++ Standard | ISO C++17 | C++17 (`-std=c++17`) | ISO Standard | Guarantees standard library features (`std::filesystem`, `std::optional`, `std::variant`, `std::string_view`, structured bindings) across MSVC, GCC, and Clang on Windows 11 x64. | C++20 if required by specific networking framework. |
| **DDR-02** | C++ HTTP Server Engine | Drogon / `cpp-httplib` | Drogon v1.9+ or `cpp-httplib` v0.15+ | MIT / BSD | **Drogon:** Asynchronous, high-throughput, non-blocking C++ framework recommended by prompt. **Trade-off:** Drogon on Windows MinGW requires Trantor, c-ares, and JsonCpp, which can introduce external DLL friction in offline packaging.<br>**Decision:** Implement an `IHttpServer` abstraction layer. Evaluate Drogon build; if build complexities on MinGW emerge, utilize `cpp-httplib` (single-header, multi-threaded C++17, zero external DLLs, built-in static file routing) as the documented, reliable zero-admin fallback. | Clean fallback to `cpp-httplib` verified offline with static linking. |
| **DDR-03** | JSON Serialization | `nlohmann/json` | v3.11.3 | MIT License | De-facto industry standard for modern C++ JSON serialization. Single-header (`json.hpp`), zero external dependencies, robust parsing with clear exceptions and type safety. | `jsoncpp` (MIT). |
| **DDR-04** | C++ Testing Suite | GoogleTest | v1.14.0 | BSD-3-Clause | Standard C++ unit testing and mocking framework. Integrates directly into CMake and CTest for automated pass/fail verification and CI reporting. | Header-only test harness fallback if CMake FetchContent is offline. |
| **DDR-05** | Linear Algebra & Numerical Solvers | Custom C++17 Numerical Algorithms (STL) | N/A (Standard Library) | ISO C++17 | The 1D steady-state FEA problem produces a symmetric positive-definite tridiagonal system $\mathbf{K}\mathbf{T} = \mathbf{F}$. A dedicated Thomas algorithm ($O(N)$ complexity) or Gaussian elimination with partial pivoting eliminates heavy external solver dependencies (Eigen/LAPACK) while guaranteeing full mathematical transparency. | Header-only Eigen 3.4 (MPL2) if dense matrix benchmarks require it. |
| **DDR-06** | Frontend Framework | Next.js (App Router) + React | Next.js 14.2+ / React 18+ | MIT License | Industry-standard React meta-framework. Configured with `output: 'export'` to generate pre-rendered static HTML/CSS/JS assets served by the C++ backend on loopback. Zero Node.js runtime required on target machine. | Bundled portable Node.js runtime if static export was incompatible (not needed; full client-side dynamic state with C++ REST API works seamlessly). |
| **DDR-07** | Styling & UI System | Tailwind CSS + Lucide Icons | Tailwind v3.4+ / Lucide React | MIT / ISC | Utility-first CSS providing a restrained, professional engineering dark/neutral theme with strict typography, high contrast, and responsive layout. | Custom CSS variables. |
| **DDR-08** | 3D Visualization | Three.js + OrbitControls | Three.js r128+ | MIT License | Hardware-accelerated WebGL rendering for 3D ASCII STL triangle meshes (wireframe, solid, facet normals) and ANSYS nodal temperature point clouds with custom colormaps (Viridis/Jet) and raycasting node inspection. | Canvas 2D isometric projection fallback. |
| **DDR-09** | 2D Engineering Charting | SVG / Chart.js / Canvas Charting | Chart.js 4+ or Custom SVG Engine | MIT License | Lightweight, interactive plotting of FEA temperature distributions, analytical curves, mesh convergence rates, benchmark speedup/efficiency, and neural loss decay. | Clean custom SVG charts with exportable raw CSV. |
| **DDR-10** | Windows Launcher | Win32 C++ Native Launcher (`Launch.exe`) | Win32 API (C++17) | Proprietary / MIT | Zero external runtime dependencies. Compiled to a standalone native binary that uses Windows Sockets (`ws2_32`) for ephemeral port acquisition, creates child processes assigned to Windows Job Objects, and invokes the default system browser via `ShellExecuteW`. | PowerShell launch script (as documented fallback, though master prompt specifies double-clickable `Launch.exe`). |

---

### Licensing Compliance Summary
All selected libraries and dependencies are published under permissive open-source licenses:
- **MIT License:** `nlohmann/json`, `cpp-httplib`, `Next.js`, `React`, `Tailwind CSS`, `Three.js`, `Chart.js`.
- **BSD-3-Clause:** `GoogleTest`, `Drogon`.
- **ISC License:** `Lucide React`.

No copyleft (GPL / AGPL) libraries are utilized. Redistribution within `CandidateID_Workbench.zip` complies strictly with all third-party license terms, with full copyright notices and licenses retained in `docs/reference/THIRD_PARTY_LICENSES.md`.

