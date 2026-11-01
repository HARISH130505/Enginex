# Statement of Ownership & Attribution
## ENGINEX — Unified Engineering Simulation & Analysis Platform (ENGINEX Workbench)

### 1. Statement of Authorship
This software project, **ENGINEX Workbench**, is authored for the **PASS Science Works R&D Internship 2026–2027** evaluation. All original architectural designs, C++ core numerical solvers, multithreaded benchmarking routines, device adapter abstractions, neural backpropagation engines, frontend components, and documentation were conceived, designed, implemented, and verified specifically for this assignment.

---

### 2. Attribution of Third-Party Libraries & Dependencies

| Library / Tool | Primary Author / Organization | License | Usage in Project |
| :--- | :--- | :--- | :--- |
| **Next.js** | Vercel, Inc. | MIT License | Frontend framework for UI structure, routing, and static asset generation. |
| **React** | Meta Platforms, Inc. | MIT License | Component-based client rendering engine. |
| **Tailwind CSS** | Tailwind Labs, Inc. | MIT License | Utility-first styling framework for engineering dashboard layout. |
| **Three.js** | Ricardo Cabello (mrdoob) & contributors | MIT License | WebGL 3D rendering for ASCII STL triangle meshes and nodal temperature point clouds. |
| **Lucide Icons** | Lucide Contributors | ISC License | Accessible, consistent vector iconography for UI controls and status badges. |
| **nlohmann/json** | Niels Lohmann | MIT License | Single-header C++17 JSON serialization and deserialization library. |
| **cpp-httplib** | Yuji Hirose | MIT License | Standalone, multi-threaded C++17 HTTP/REST server library for loopback API serving. |
| **GoogleTest** | Google LLC | BSD-3-Clause | Unit testing and assertion framework for C++ core verification. |

No proprietary, commercial, closed-source, or copyleft (GPL/AGPL) codebases are incorporated.

---

### 3. Data Provenance & Synthetic Generation Notice
In strict accordance with the assignment rules:
1. **ASCII STL Geometry:** The test geometry files in `data/geometry/` are generated synthetically or derived from open, public-domain engineering test primitives (e.g. standard cuboid, cylinder, or bracket shapes). No proprietary CAD models are utilized. Coordinate units are documented as millimeters ($mm$) or meters ($m$) according to the specific fixture manifest.
2. **ANSYS Temperature CSV Export:** The nodal temperature dataset in `data/temperature/` is strictly synthetic and generated to validate the exact required column schema: `node_id,x_m,y_m,z_m,temperature_C`. It is explicitly labeled **SYNTHETIC** in all UI views and documentation, and is not represented as proprietary ANSYS output.
3. **Hardware Telemetry:** All telemetry packets, status codes, register reads, and connection states from the Microcontroller, FPGA, and JTAG adapters are generated via C++ mock simulation models and are explicitly displayed with a **SIMULATED** badge in the user interface. No physical hardware connection is claimed.

