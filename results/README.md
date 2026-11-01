# Results and Verification Evidence

This index describes the result artifacts currently present in the repository. It distinguishes stored outputs and logs from the latest regression results reported for the project. This documentation pass did not rerun application tests.

## Stored numerical outputs

| Output | Location | Contents |
|---|---|---|
| Thermal convergence | `results/csv/thermal_convergence.csv` | Four rows for `N = 10, 20, 40, 80`; midpoint errors are 1.25, 0.3125, 0.078125, and 0.01953125 °C. |
| Parallel benchmark | `results/benchmarks/benchmark_results.csv` | Five timed repetitions for 1–4 workers, median, speedup, efficiency, and maximum serial/parallel discrepancy. Recorded discrepancy is 0 °C for every worker count. |
| Backpropagation training | `results/csv/backpropagation_training.csv` | 101 rows for steps 0–100; rounded CSV values. |
| Backpropagation reference precision | `docs/reference/backpropagation_training_100_steps.csv` | 101 rows with higher-precision training values. |

The higher-precision training CSV records:

| Step | Loss | Prediction (`y_hat`) |
|---:|---:|---:|
| 0 | 1.1825418189970167 | 0.46211715726000974 |
| 10 | 0.010211015582825693 | 1.8570943277345109 |
| 50 | 1.2760769996583145e-10 | 1.9999840245375675 |
| 100 | 1.70262e-20 (rounded in the test log) | approximately 2.0 |

The rounded release CSV may display the step-50 loss as `0.00000000`; use the higher-precision reference CSV when reporting that milestone.

## Plot artifacts

`results/plots/` currently contains no standalone plot files. Interactive charts are implemented in the frontend, including `frontend/components/thermal/ThermalChart.tsx`, `frontend/components/performance/SpeedupChart.tsx`, and `frontend/components/neural/LossChart.tsx`. Exported, labelled plot images have not been found.

## Test and integration evidence

The latest available local CTest log is `build/Testing/Temporary/LastTest.log`, timestamped **1 November 2026 at 11:30 IST**. It records **9/9 CTest tests passing**, including `JobIntegrationTest`. The log contains development-machine paths and is not included in the release ZIP.

The latest regression summary supplied for this project additionally reports:

- Frontend typecheck, lint, and production build passed.
- Thermal meshes `N = 10, 20, 40, 80` passed; invalid thermal input returned HTTP 400 before job creation.
- Asynchronous thermal and performance jobs completed and their results were retrieved.
- Benchmark numerical agreement was within `1e-8 °C`.
- Hardware mock-device operations passed.
- Backpropagation and finite-difference gradient checks passed.
- Cancellation and error handling were tested.

Those frontend and live-integration results were reported by the project owner; supporting command transcripts for them were not found in the repository during this audit. They were not rerun for this index. No clean Windows 11/offline-machine test has been performed.

## Evidence gaps

- The benchmark CSV does not record a complete environment manifest (CPU model, RAM, physical and logical core counts, compiler/version, OS/build configuration, and optimization flags).
- No standalone plot files, frontend test logs, live-API result log, or clean-machine test log were found under `results/`.
- The release archive contains the CSV results but not this evidence index, the test log, or the project source/tests.
