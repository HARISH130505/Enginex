# Packaging Strategy and Risk Register
## ENGINEX Workbench

## 1. Package scope and validation status

The target platform is Windows 11 x64. Clean-machine, no-admin, and offline compatibility are design goals, **not verified results**; no separate clean Windows 11/offline-machine test has been performed.

The current `dist/CandidateID_Workbench.zip` is a runtime application bundle. Its inspected structure is:

```text
CandidateID_Workbench/
|-- Launch.exe
|-- README.txt
|-- libgcc_s_seh-1.dll
|-- libstdc++-6.dll
|-- libwinpthread-1.dll
|-- bin/
|   |-- backend_server.exe
|   |-- libgcc_s_seh-1.dll
|   |-- libstdc++-6.dll
|   `-- libwinpthread-1.dll
|-- web/                     # Static export, including route/index.html pages
|-- data/                    # Sample STL and synthetic temperature CSV
`-- results/                 # Thermal, benchmark, and backpropagation CSV outputs
```

The archive does not currently contain application source, tests, CMake files, dependency lock/version information, editor configuration, project documentation, screenshots/standalone plots, or Git history/bundle. Check [`../../final-submission-checklist.md`](../../final-submission-checklist.md) and the official assignment before deciding whether to expand it. The distribution README inside the current archive predates the updated packaging README source.

The packaging script inspects executable imports using `objdump` from the CMake-configured compiler installation, locates non-system dependencies in that compiler's `bin` directory, copies them beside the applicable executable, and checks packaged imports. This verifies the script's dependency-discovery path and archive contents, not operation on a clean Windows installation.

## 2. Risk register

| ID | Risk | Mitigation or current status | Acceptance evidence |
|---|---|---|---|
| RSK-01 | Port collision | Launcher requests an available ephemeral loopback port rather than assuming a fixed port. | Test concurrent launch and port allocation if required. |
| RSK-02 | Missing runtime DLL | Packaging script obtains imports from the configured compiler and checks dependency files beside the executables. Current ZIP contains the inspected MinGW runtime DLLs. | Clean Windows 11 startup remains untested. |
| RSK-03 | Local firewall/security policy | Backend is designed to bind to loopback. Do not assume target firewall behavior. | Test on the target environment if required. |
| RSK-04 | Static export route mismatch | Frontend uses a static export; exported routes are stored as route `index.html` pages. | Routes were previously checked through the packaged static server; see `results/README.md` for evidence limits. |
| RSK-05 | Orphaned backend process | Launcher associates the backend with a Windows Job Object for lifecycle control. | Latest regression summary does not establish a separate clean-machine test. |
| RSK-06 | Paths containing spaces or Unicode | Launcher resolves paths from its executable location and uses Win32 APIs. | Test with representative extraction paths if required. |
| RSK-07 | Numerical drift due to optimization | Benchmark output records serial/parallel discrepancy; latest CSV reports zero discrepancy. | Capture compiler flags and environment in the benchmark manifest. |
| RSK-08 | Concurrency defects | Bounded worker pool, deterministic result indexing, and CTest coverage are implemented. | Latest local CTest log records 9/9 tests passing; stress behavior beyond those tests is not claimed. |

These are engineering risks and acceptance suggestions, not claims that every listed scenario has been tested.
