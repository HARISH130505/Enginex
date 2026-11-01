# ENGINEX — Unified Engineering Simulation & Analysis Platform
### Application: ENGINEX Workbench
**PASS Science Works R&D Internship 2026–2027 Evaluation Project**

[![Platform: Windows 11 x64](https://img.shields.io/badge/Platform-Windows%2011%20x64-blue.svg)](https://microsoft.com)
[![Backend: C++17](https://img.shields.io/badge/Backend-C%2B%2B17-00599C.svg?logo=c%2B%2B)](https://isocpp.org)
[![Frontend: Next.js + React](https://img.shields.io/badge/Frontend-Next.js%20%2B%20TypeScript-black.svg?logo=next.js)](https://nextjs.org)
[![Build: CMake](https://img.shields.io/badge/Build-CMake-064F8C.svg?logo=cmake)](https://cmake.org)
[![License: Permissive Open-Source](https://img.shields.io/badge/License-MIT%20%2F%20BSD-green.svg)](docs/ownership-statement.md)

---

## 1. Overview
**ENGINEX Workbench** is an integrated desktop engineering simulation and analysis suite uniting computer-aided design (CAD) visualization, finite-element thermal analysis, multithreaded high-performance computing, embedded hardware telemetry simulation, and handwritten neural backpropagation into one cohesive, single-origin interface.

### The Five Core Engineering Views
1. **Geometry & CAD Inspector (`/geometry`):** 3D ASCII STL mesh inspection with spatial bounding boxes and 3D colored point-cloud rendering of ANSYS nodal temperature distributions (`node_id,x_m,y_m,z_m,temperature_C`) with temperature legend and node inspection.
2. **Thermal Analysis (`/thermal`):** Exact 1D steady-state finite-element solver for a uniform heat-conducting rod with uniform volumetric heat generation ($qA$), comparing piecewise linear FE approximations against the exact analytical solution across $N \in \{10, 20, 40, 80\}$ elements with midpoint error interpolation.
3. **Multicore Performance (`/performance`):** Reproducible serial and parallel batch simulation benchmarks across 1–4 CPU workers with deterministic input-order result preservation, speedup and efficiency calculation, and strict numerical agreement verification ($\max \Delta T \le 10^{-8\ \circ}\text{C}$).
4. **Embedded Hardware Workbench (`/hardware`):** Host-connected integration design and C++ mock adapters for Microcontroller (ARM Cortex-M4), FPGA (Lattice iCE40UP5K), and JTAG debug interface (FT2232H), demonstrating real-time simulated telemetry, fault injection (timeout), and state recovery.
5. **Neural Backpropagation (`/backpropagation`):** Single-sample regression training step ($x=1, y=2$) with analytical gradient derivations, central finite-difference verification ($\epsilon = 10^{-6}$, error $\le 10^{-5}$), and 100-step training loss history.

---

## 2. End-User Runtime Instructions (Zero-Admin / Turnkey)
The packaged distribution `CandidateID_Workbench.zip` requires **no administrative rights, no compiler, no Node.js/Python, and no internet connection**.

1. Extract `CandidateID_Workbench.zip` to any folder on a Windows 11 x64 machine.
2. Double-click `Launch.exe`.
3. The launcher will automatically:
   - Identify an available ephemeral loopback TCP port (`127.0.0.1`).
   - Launch the local C++ calculation and REST service in the background.
   - Await backend health readiness confirmation (`/api/v1/health`).
   - Open your default web browser to the workbench dashboard.
4. **Clean Exit:** To exit the workbench, close the launcher application window or press `Ctrl+C` in the launcher console. Windows Job Objects guarantee that all background backend processes are terminated cleanly without leaving orphaned processes.

---

## 3. Developer Prerequisites & Source Build

### Prerequisites (Development Environment Only)
- **Operating System:** Windows 11 x64
- **C++ Compiler:** GCC 13+ (MinGW-w64) or MSVC 2022 (with C++17 support)
- **Build System:** CMake 3.20+ and Ninja
- **Node.js:** v18.0+ or v20.0+ (used during development to build and export static web assets)
- **Git:** 2.30+

### Building C++ Core and Test Suite
```powershell
# Configure CMake build with Ninja
cmake -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release

# Compile all libraries, test targets, and server binaries
cmake --build build --config Release

# Execute full GoogleTest + CTest verification suite
ctest --test-dir build --output-on-failure
```

### Building & Statically Exporting Frontend
```powershell
cd frontend
npm install
npm run build
# Generates statically exported assets into frontend/out/
```

---

## 4. Architecture & Module Boundaries
```
Enginex/
├── backend/                     # ISO C++17 domain engines and local REST server
│   ├── include/workbench/       # Public header interfaces
│   │   ├── core/                # Result<T>, JobManager, ThreadPool
│   │   ├── geometry/            # StlParser, CsvParser
│   │   ├── thermal/             # Solver1D, MeshStudy
│   │   ├── performance/         # BatchGenerator, BenchmarkHarness
│   │   ├── hardware/            # IDeviceAdapter, MockAdapters
│   │   ├── neural/              # BackpropCore
│   │   └── api/                 # HttpServer, RestRoutes
│   └── src/                     # C++17 implementation files
├── frontend/                    # Next.js App Router + TypeScript + Tailwind UI
│   ├── app/                     # 5 routed views: geometry, thermal, performance, hardware, backpropagation
│   └── components/              # Three.js 3D viewers, SVG charts, status cards
├── data/                        # Verified synthetic fixtures (STL geometry, CSV nodal temps)
├── tests/                       # Unit, integration, and negative test fixtures
├── results/                     # Raw exported CSVs, plots, and benchmark logs
├── docs/                        # Architecture, RTM, Risk Register, 15-day plan, AI log, Report
├── packaging/                   # Win32 Launch.exe source, bundling scripts
└── CMakeLists.txt               # Root multi-target build configuration
```

---

## 5. Phase Execution Status
- [x] **Phase G0 — Discovery & Architecture:** Requirements extracted, RTM complete, DDR locked, Risk Register defined, 15-day plan committed.
- [x] **Phase G1 — Skeleton & Build Pipeline:** CMake scaffold, C++ core, API server, Next.js static build, health check.
- [x] **Phase G2 — Geometry & Data Visualization:** ASCII STL & CSV parsers, Three.js 3D viewers.
- [x] **Phase G3 — 1D Thermal FEA Solver:** Rod formulation, assembly, exact analytical comparison, midpoint error, mesh study.
- [x] **Phase G4 — Multicore Benchmarking:** Thread pool, batch generator, 1–4 workers, 5 repetitions, medians, speedup, agreement.
- [x] **Phase G5 — Embedded System Integration:** Block diagram, pin-voltage table, 3 mock adapters, simulated timeout & recovery.
- [x] **Phase G6 — Handwritten Backpropagation:** Analytical gradients, central finite differences ($\le 10^{-5}$), 100-step loss curve.
- [x] **Phase G7 — System Integration & UX:** Unified navigation, responsive status, cancellation, CSV export.
- [x] **Phase G8 — Release & Verification:** Standalone `Launch.exe`, ZIP packaging, clean-machine verification, technical report.

