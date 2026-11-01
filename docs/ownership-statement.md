# Ownership, Attribution, and Provenance

## 1. Project Ownership and Candidate Contributions

ENGINEX — Unified Engineering Simulation & Analysis Platform was developed as part of the PASS Science Works R&D Internship evaluation assignment.

The candidate is responsible for the project-specific design decisions, implementation, integration, testing, verification, and preparation of the submission materials.

AI development assistants, including Antigravity and GitHub Copilot, were used during planning, implementation, debugging, testing support, and documentation. Their use and the areas of assistance are described in the accompanying `ai-use-log.md`.

The candidate reviewed AI-generated suggestions, made implementation decisions, and takes responsibility for understanding and explaining the final submitted work.

## 2. Third-Party Software and Dependencies

The project uses third-party software libraries and development dependencies.

- Frontend direct dependencies and version ranges are listed in `frontend/package.json`.
- Resolved frontend dependency versions are recorded in `frontend/package-lock.json`.
- C++ dependencies and build configuration are maintained in the backend and CMake build configuration.

Third-party software remains subject to its respective licenses. Required license notices and redistribution conditions must be respected.

A complete software bill of materials and transitive dependency license audit have not been established in this document. Refer to the relevant package and license files when reviewing redistribution requirements.

## 3. Sample Data and Provenance

The project includes sample files for demonstrating geometry and temperature-data visualisation.

- `data/temperature/synthetic_ansys_sample.csv`: A synthetic temperature-data fixture following the application's nodal-temperature CSV format. It is not presented as an actual ANSYS export.
- `data/geometry/sample_cube.stl`: A sample cube geometry fixture used to demonstrate STL import and visualisation. Its original creator and source should be documented once confirmed.

The sample files are included for application demonstration and testing. Their origins should not be misrepresented as externally sourced or independently created without supporting evidence.

## 4. Embedded Hardware Attribution

The MCU, FPGA, and debug-adapter interfaces in ENGINEX are implemented as software simulations using mock adapters.

The application demonstrates device discovery, connection, telemetry, disconnection, and simulated fault handling. No physical hardware connection, electrical validation, or real device communication is claimed.

## 5. Handwritten Backpropagation Derivation

The handwritten backpropagation derivation is prepared separately as part of the assignment requirements. It documents the forward pass, chain-rule gradients, simultaneous parameter update, and gradient-direction explanation.

The handwritten derivation is to be included as a separate scanned submission attachment.

## 6. Candidate Declaration

I confirm that I have reviewed this ownership, attribution, and provenance statement and that the descriptions of my contributions, AI assistance, third-party materials, and sample data are accurate to the best of my knowledge.

I understand that third-party libraries and materials remain subject to their respective ownership and licensing terms.

**Harish J** 
**01-11-2026** 