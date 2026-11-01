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
  status: "PENDING" | "RUNNING" | "COMPLETED" | "CANCELLED" | "FAILED";
  progress: number;
  status_message: string;
  cancel_requested: boolean;
  result?: any;
  error_message?: string;
  created_at: string;
  completed_at?: string;
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

export async function cancelJob(jobId: string): Promise<void> {
  const base = getApiBaseUrl();
  const res = await fetch(`${base}/api/v1/jobs/${jobId}/cancel`, {
    method: "POST",
  });
  if (!res.ok) {
    throw new Error(`Failed to cancel job ${jobId}: ${res.status}`);
  }
}

