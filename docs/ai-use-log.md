# AI Usage Disclosure and Verification Log

## ENGINEX — Unified Engineering Simulation & Analysis Platform

**Application:** ENGINEX Workbench

## 1. Purpose and Scope

AI tools were used as development assistants during project planning, implementation, debugging, testing support, integration, and documentation.

AI-generated suggestions were reviewed rather than treated as authoritative. The candidate made project decisions, evaluated proposed changes, and used builds, automated tests, numerical checks, API checks, and manual application testing to assess the resulting work.

This log summarises the Antigravity and Copilot-assisted work recorded for the project. The Antigravity entries are based on an earlier project interaction summary; complete original prompts and transcripts are not reproduced here. Model names are included as previously recorded. Copilot model/version information was not available in the session history.

## 2. AI Tools Used

| AI Tool | Model Details Recorded | Role |
|---|---|---|
| Antigravity Pair-Programming Assistant | Gemini 3.8 Flash (Medium) for most phases; Claude Sonnet 4.6 (Thinking) for multicore benchmarking, as recorded in the earlier log. | Planning, scaffolding, geometry, thermal solver, benchmarking, embedded mocks, backpropagation, integration and packaging. |
| GitHub Copilot in VS Code | Model/version not exposed in the available session history. | Packaging dependency investigation, thermal validation, frontend/static-export debugging, regression support and documentation review. |

## 3. Summary of AI-Assisted Development

### 3.1 Architecture and Core Application

**AI assistance:**
- Assignment analysis and phased project planning.
- C++17 backend and Next.js static frontend scaffolding.
- Loopback HTTP server and launcher integration.

**Candidate review and responsibility:**

The candidate selected `cpp-httplib` and `nlohmann/json` instead of the initially considered Drogon option and retained the five distinct engineering views.

### 3.2 Geometry and Visualisation

**AI assistance:**
- ASCII STL and nodal-temperature CSV parsing.
- Input validation and error handling.
- Three.js geometry and point-cloud viewer components.

**Candidate review and responsibility:**

The candidate reviewed parser behaviour, invalid-input handling and the displayed geometry and point-cloud data.

### 3.3 Thermal Finite-Element Analysis

**AI assistance:**
- One-dimensional steady-state thermal solver.
- Boundary-condition handling.
- Mesh study, verification tests and result presentation.

**Candidate review and responsibility:**

The candidate reviewed the boundary-condition formulation and compared numerical results with the analytical solution for N = 10, 20, 40 and 80.

### 3.4 Multicore Performance Benchmarking

**AI assistance:**
- Seeded batch generation.
- Bounded thread-pool implementation.
- Repeated benchmark measurements.
- Speedup and efficiency calculations.

**Candidate review and responsibility:**

The candidate reviewed numerical agreement and reported small-workload slowdowns where thread overhead dominated rather than presenting them as performance gains.

### 3.5 Embedded-System Simulation

**AI assistance:**
- Mock MCU, FPGA and debug-adapter interfaces.
- Telemetry and device operations.
- Fault injection and recovery.
- Supporting design documentation.

**Candidate review and responsibility:**

The candidate ensured that device behaviour was clearly labelled as simulated. No physical hardware connection or electrical testing is claimed.

### 3.6 Backpropagation

**AI assistance:**
- Single-sample C++ implementation.
- Analytical gradient calculations.
- Central finite-difference verification.
- Loss-history visualisation.

**Candidate review and responsibility:**

The candidate reviewed numerical gradient checks and prepared the required handwritten derivation separately.

### 3.7 Integration and Release Packaging

**AI assistance:**
- Dashboard integration.
- Runtime dependency packaging.
- Thermal request validation.
- Static-export routing diagnosis.
- Regression support and documentation.

**Candidate review and responsibility:**

The candidate reviewed changes and packaged application behaviour. Clean-machine compatibility was not claimed because a separate clean Windows 11/offline-machine test was not performed.

## 4. Critical Review of an Unsuitable AI Suggestion

**Topic: Dirichlet boundary conditions in the thermal finite-element solver.**

The earlier project log records a preliminary suggestion to zero the boundary rows and set boundary values without eliminating the corresponding boundary columns or adjusting the interior right-hand-side entries.

That treatment would leave prescribed-boundary contributions in the interior equations.

The candidate identified the issue and used symmetric Dirichlet elimination:

- Adjust the interior load vector for known boundary-temperature contributions.
- Zero the boundary row and column.
- Set the boundary diagonal to one.
- Set the boundary load to the prescribed temperature.

The solver was subsequently checked against analytical thermal results and the mesh study.

This example is carried forward from the earlier project log; the full original assistant response and transcript are not included in this summary.

## 5. Verification Evidence

The latest recorded project regression results include:

- **C++ test suite:** 9 passed, 0 failed. The available `LastTest.log` records nine passing CTest entries on 1 November 2026.
- **Frontend:** Type checking, linting and production build reported as passed.
- **Thermal analysis:** Meshes N = 10, 20, 40 and 80 checked. Invalid thermal requests, including excessive mesh input, returned HTTP 400 without creating jobs.
- **Asynchronous workflows:** Thermal and benchmark jobs completed and results were retrieved. Serial/parallel numerical agreement was within `1e-8`.
- **Hardware mocks:** Device listing, connection, telemetry and disconnection were exercised. Fault behaviour remained simulated.
- **Backpropagation:** Training and all four central finite-difference gradient checks reported as passed.
- **Packaging:** Release ZIP extracted to a separate temporary directory; launcher, backend, health endpoint and principal application workflows were exercised.

The CTest count is supported by the available test log. Frontend and live-integration results are from the candidate-supplied regression summary; separate detailed transcripts for every check were not available in the documentation audit.

The documentation-only Copilot review did not rerun the application build or tests.

## 6. Limitations

- No physical MCU, FPGA or debug adapter was connected or electrically tested. Embedded behaviour is simulated.
- A separate clean Windows 11/offline-machine installation test has not been performed.
- The available Copilot session history does not expose its model/version.
- The Antigravity model labels and interaction summaries are reproduced from the earlier project log, not from complete original transcripts.

## 7. Candidate Acknowledgement

I have reviewed this AI Usage Disclosure and confirm that it accurately describes the AI tools and assistance used in the project to the best of my knowledge.

I understand that I remain responsible for the final submitted implementation, results and documentation.

**Harish J**
**01-11-2026** 