// Dynamic API Base URL Discovery
// When loaded via C++ loopback HTTP server, window.location.origin points directly to the server.
export function getApiBaseUrl(): string {
  if (typeof window !== "undefined") {
    // In browser environment, use current host/port
    return window.location.origin;
  }
  return "http://127.0.0.1:8080";
}

export interface HealthResponse {
  status: string;
  application: string;
  project: string;
  version: string;
  backend: string;
  platform: string;
  uptime_seconds: number;
  bound_port: number;
  routes: string[];
}

export interface JobResponse {
  id: string;
  type: string;
  status: "QUEUED" | "PENDING" | "RUNNING" | "COMPLETED" | "CANCELLED" | "FAILED";
  progress: number;
  status_message: string;
  cancel_requested: boolean;
  result?: unknown;
  error_message?: string;
  created_at: string;
  started_at?: string;
  completed_at?: string;
}

export interface JobSubmitResponse {
  job_id: string;
  status: string;
  type: string;
  message: string;
}

export async function fetchHealth(): Promise<HealthResponse> {
  const base = getApiBaseUrl();
  const res = await fetch(`${base}/api/v1/health`, {
    cache: "no-store",
  });
  if (!res.ok) {
    throw new Error(`Health check failed with status: ${res.status}`);
  }
  return res.json();
}

export async function fetchJob(jobId: string): Promise<JobResponse> {
  const base = getApiBaseUrl();
  const res = await fetch(`${base}/api/v1/jobs/${jobId}`, {
    cache: "no-store",
  });
  if (!res.ok) {
    throw new Error(`Failed to fetch job ${jobId}: ${res.status}`);
  }
  return res.json();
}

export interface ThermalSolution {
  N: number;
  h: number;
  x_nodes: number[];
  T_fe: number[];
  T_exact: number[];
  max_midpoint_error: number;
}

export interface ThermalStudyResult {
  csv_export: string;
  solutions: ThermalSolution[];
}

export interface BenchmarkWorkerResult {
  workers: number;
  rep_durations_ms: number[];
  median_ms: number;
  speedup: number;
  efficiency: number;
  max_temp_diff_vs_serial: number;
}

export interface PerformanceBenchmarkResult {
  csv_export: string;
  logical_cores: number;
  hardware_model: string;
  worker_results: BenchmarkWorkerResult[];
  agreement_verified: boolean;
}

export interface TrainingHistoryEntry {
  step: number;
  loss: number;
  y_hat: number;
}

export interface GradientCheckEntry {
  param_name: string;
  analytical: number;
  numerical: number;
  absolute_error: number;
  passed: boolean;
}

export interface NeuralTrainingResult {
  csv_export: string;
  history: TrainingHistoryEntry[];
  grad_check: {
    entries: GradientCheckEntry[];
    epsilon: number;
    tolerance: number;
  };
}

export function getErrorMessage(error: unknown, fallback: string): string {
  return error instanceof Error ? error.message : fallback;
}

export async function fetchJobResult(jobId: string): Promise<unknown> {
  const base = getApiBaseUrl();
  const res = await fetch(`${base}/api/v1/jobs/${jobId}/result`, {
    cache: "no-store",
  });
  if (!res.ok) {
    const err = await res.json().catch(() => ({}));
    throw new Error(err.error || `Failed to fetch result for job ${jobId}: ${res.status}`);
  }
  return res.json();
}

export async function cancelJob(jobId: string): Promise<void> {
  const base = getApiBaseUrl();
  const res = await fetch(`${base}/api/v1/jobs/${jobId}/cancel`, {
    method: "POST",
  });
  if (!res.ok) {
    const err = await res.json().catch(() => ({}));
    throw new Error(err.error || `Failed to cancel job ${jobId}: ${res.status}`);
  }
}

export async function submitThermalJob(payload: Record<string, unknown>): Promise<JobSubmitResponse> {
  const base = getApiBaseUrl();
  const res = await fetch(`${base}/api/v1/thermal/jobs`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });
  const data = await res.json();
  if (!res.ok) {
    throw new Error(data.error || "Thermal job submission failed");
  }
  return data;
}

export async function submitPerformanceJob(payload: Record<string, unknown>): Promise<JobSubmitResponse> {
  const base = getApiBaseUrl();
  const res = await fetch(`${base}/api/v1/performance/jobs`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });
  const data = await res.json();
  if (!res.ok) {
    throw new Error(data.error || "Performance job submission failed");
  }
  return data;
}
