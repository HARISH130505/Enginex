# PASS Science Works R&D Internship 2026–2027
## Master Prompt Reference & Assignment Specifications

### Key Assignment Rules
- **Full project name:** ENGINEX — Unified Engineering Simulation & Analysis Platform
- **Repository name:** Enginex
- **Application name:** ENGINEX Workbench
- **Target OS:** Windows 11 x64
- **Backend:** All backend logic, numerical computation, file processing, and concurrency must be C++17 or later.
- **Frontend:** Next.js + TypeScript + React + Tailwind CSS (static exportable client frontend).
- **Packaging:** Self-contained ZIP extraction followed by double-clicking `Launch.exe`. Zero terminal, zero admin rights, zero package installs, zero first-run compilation, fully offline.
- **Single Loopback Origin:** C++ local server serves static frontend and REST API from `127.0.0.1:<port>`.

### Core Engineering Modules
1. **Task A — Interface, Backend, and One-Click Execution:**
   - 5 routes: Geometry, Thermal Analysis, Performance, Hardware, Backpropagation.
   - Standard workflow: input selection -> validation -> run/import -> progress/status -> visualization/results -> export.
   - Non-blocking asynchronous jobs, polling, cancellation without partial result overwrite.
   - One-click `Launch.exe` with dynamic port discovery, default browser launch, and clean shutdown.

2. **Task B — Existing CAD / ANSYS Data Visualization:**
   - ASCII STL parser in C++: validates facet/outer loop/vertex syntax, finite coordinates, normals, complete triangles.
   - 3D triangle display: orbit, pan, zoom, fit-to-view, bounding box, units/coordinate assumptions.
   - ANSYS nodal temperature CSV parser in C++: schema `node_id,x_m,y_m,z_m,temperature_C`.
   - 3D coloured point cloud viewer: temperature legend, °C units, colour scale range, interactive node inspection.
   - Negative tests: empty file, missing header column, reordered columns, malformed numeric value, NaN/Inf, truncated facet, invalid vertex count, oversized file, duplicate node ID policy.

3. **Task C — Steady-State 1D Thermal Finite-Element Solver:**
   - Rod length $L = 1\text{ m}$, cross-sectional area $A = 1\times 10^{-4}\text{ m}^2$, thermal conductivity $k = 10\text{ W/(m}\cdot\text{K)}$, heat generation $q = 10{,}000\text{ W/m}^3$, $T(0) = 100^\circ\text{C}, T(L) = 20^\circ\text{C}$.
   - Uniform 1D two-node linear elements: local stiffness $(kA/h)[[1,-1],[-1,1]]$, consistent load vector $(qAh/2)[1,1]^T$.
   - Global assembly with mathematically correct Dirichlet boundary elimination.
   - Exact analytical comparison: $T_{\text{exact}}(x) = 100 - 80\frac{x}{L} + \frac{q}{2k}x(L-x)$.
   - Mesh study for $N \in \{10, 20, 40, 80\}$. Midpoint error evaluated using piecewise linear FE interpolation. Convergence table and plot. CSV export.

4. **Task D — Multicore / Multithreaded Execution:**
   - Batch generator with fixed, recorded seed, varied positive $k$ and $q$, configurable case count and mesh size.
   - Bounded worker pool (`std::thread`), deterministic output order, thread-safe result management.
   - Serial and parallel agreement check: max absolute temperature difference $\le 10^{-8\ \circ}\text{C}$.
   - Benchmark protocol: 1, 2, 3, 4 workers (capped at logical cores). Monotonic clock. Warmup + 5 repetitions per worker count. Medians, speedup $T_1/T_p$, efficiency $(T_1/T_p)/p$. Environment manifest.

5. **Task E — Embedded-System Integration Design:**
   - Host-connected workbench for 1 Microcontroller, 1 FPGA, 1 JTAG debug adapter.
   - Block diagram, pin-voltage table, level-shifting analysis, JTAG signal ownership, safe sequencing.
   - C++ mock adapters: `discover`, `capabilities`, `connect`, `disconnect`, `telemetry`.
   - Simulated timeout & retry recovery in UI. All mock devices clearly labeled SIMULATED.

6. **Task F — Handwritten Backpropagation and C++ Implementation:**
   - Regression sample $x=1, y=2, h=\tanh(wx+b), \hat{y}=vh+c, J=0.5(\hat{y}-y)^2$. Initial $w=0.5, b=0, v=1, c=0, \eta=0.1$.
   - Theoretical values: $h = \tanh(0.5) \approx 0.462117$, $\hat{y} \approx 0.462117$, $e \approx -1.537883$, $J \approx 1.182542$.
   - Analytical gradients: $\partial J/\partial w \approx -1.209464$, $\partial J/\partial b \approx -1.209464$, $\partial J/\partial v \approx -0.710682$, $\partial J/\partial c \approx -1.537883$.
   - Updated params: $w \approx 0.620946$, $b \approx 0.120946$, $v \approx 1.071068$, $c \approx 0.153788$.
   - C++ central finite differences check ($\epsilon = 10^{-6}$): absolute error $\le 10^{-5}$ for each parameter.
   - 100-step training loss history and curve plot.

