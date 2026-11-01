# Packaging Strategy & Risk Register
## ENGINEX — Unified Engineering Simulation & Analysis Platform (ENGINEX Workbench)

### 1. Zero-Admin Turnkey Packaging Architecture

The target operating environment is a clean Windows 11 x64 installation with no compiler toolchains, no Node.js/npm runtime, no Python, and no administrative privileges. The deliverable is a self-contained archive `CandidateID_Workbench.zip`.

```
CandidateID_Workbench/
├── Launch.exe                   # Zero-dependency Win32 launcher executable
├── bin/
│   ├── backend_server.exe       # C++17 Local HTTP & Core Calculation Engine
│   ├── libgcc_s_seh-1.dll       # MinGW-w64 runtime (if dynamic) or statically linked
│   ├── libstdc++-6.dll          # MinGW-w64 C++ runtime
│   └── libwinpthread-1.dll      # POSIX threads runtime
├── web/                         # Statically exported Next.js client (HTML, CSS, JS)
│   ├── index.html
│   ├── geometry.html
│   ├── thermal.html
│   ├── performance.html
│   ├── hardware.html
│   ├── backpropagation.html
│   └── _next/
├── data/                        # Sample data fixtures (STL, CSV)
│   ├── geometry/
│   └── temperature/
├── results/                     # Pre-generated benchmark & simulation outputs
├── docs/                        # Complete technical documentation suite
└── README.md                    # Quickstart & evaluation instructions
```

---

### 2. Comprehensive Risk Register & Mitigation Matrix

| Risk ID | Risk Description | Severity | Probability | Impact | Mitigation Strategy | Verification / Acceptance Gate |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **RSK-01** | **Port Collision on Target Machine:** Hardcoding port 8080 or 3000 causes immediate startup failure if another service is bound. | Critical | High | Application fails to start on evaluator's machine; launch gate fails. | `Launch.exe` queries OS kernel for an ephemeral loopback port (`bind(sock, 127.0.0.1:0)`), reads assigned port via `getsockname`, and passes `--port <P>` to `backend_server.exe`. | Test concurrently launching with port 8080 occupied by another process. |
| **RSK-02** | **Missing C++ Runtime DLLs on Clean Windows:** Running a dynamically linked binary on a clean machine fails with `VCRUNTIME140.dll` or `libstdc++-6.dll not found`. | Critical | High | Instant DLL load error dialog upon double-clicking `Launch.exe`. | Compile with static runtime flags (`-static -static-libgcc -static-libstdc++`) or bundle verified redistributable DLLs in the same directory as the executable. | Test on a fresh Windows 11 sandbox/VM without development tools. |
| **RSK-03** | **Windows Firewall Inbound Pop-up:** Evaluator without admin rights cannot accept firewall security prompts. | High | Medium | App launch blocked or network prompt intimidates evaluator. | Bind the local C++ server strictly to `127.0.0.1` (loopback). Windows Firewall does not inspect or restrict loopback traffic. | Verify that no Windows Defender alert triggers during launch. |
| **RSK-04** | **Next.js Dynamic Routing / SSR Incompatibility with Static Export:** Using dynamic server functions breaks static HTML export. | High | Medium | Build failure during static export (`next build`). | Enforce purely client-side state handling with React hooks (`useState`, `useEffect`) and static export (`output: 'export'`, `trailingSlash: true`). Frontend discovers API origin from `window.location.origin`. | Verify `npm run build` generates pure static files into `frontend/out/`. |
| **RSK-05** | **Orphan Server Background Processes:** Closing the browser leaves `backend_server.exe` running indefinitely in background. | Medium | High | Lingering background processes consume CPU/RAM and hold ports. | `Launch.exe` places the spawned backend process inside a Windows Job Object configured with `JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE`. When `Launch.exe` terminates, Windows automatically terminates all descendant processes. | Terminate `Launch.exe` via Task Manager; verify backend process exits immediately. |
| **RSK-06** | **Path Traversal with Spaces or Unicode:** Evaluator extracts ZIP to a path containing spaces (e.g. `C:\Users\John Doe\OneDrive - Corp\Desktop\`). | Medium | High | Command execution or asset loading fails due to unquoted file paths. | All Win32 API calls (`CreateProcessW`, `ShellExecuteW`) use wide-character UTF-16 APIs with strict quote wrapping. Path resolution uses `GetModuleFileNameW` to determine directory relative to executable. | Test running from folder paths with spaces, brackets, and Unicode characters. |
| **RSK-07** | **Compiler Aggressive Optimization (`-ffast-math`) altering FEA results:** Non-IEEE floating point breaks exact benchmark comparisons. | High | Low | Serial vs parallel results drift beyond $10^{-8\ \circ}\text{C}$ tolerance. | Strictly avoid `-ffast-math`. Use standard IEEE 754 double precision (`double`), strictly reproducible operations, and exact analytical equations. | Run unit test `test_concurrency_agreement` in Release mode. |
| **RSK-08** | **Race Conditions in Thread Pool Batch Solver:** Shared mutable state causes non-deterministic results or memory corruption. | Critical | Medium | Random test failures, non-deterministic output order, crash under load. | Each task receives an independent copy of thermal boundary/material conditions and its own result buffer. Worker pool uses thread-safe mutex and condition variables with task index preservation. | Stress test with 1000 tasks across 16 threads; assert deterministic result order. |
