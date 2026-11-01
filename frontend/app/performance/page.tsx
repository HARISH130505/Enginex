"use client";

import React, { useState } from "react";
import { 
  Cpu, 
  Play, 
  Download, 
  TrendingUp, 
  CheckCircle2, 
  ShieldCheck, 
  AlertTriangle, 
  Loader2,
  HardDrive,
  Info
} from "lucide-react";
import { SpeedupChart } from "@/components/performance/SpeedupChart";
import { BenchmarkTable } from "@/components/performance/BenchmarkTable";
import { getApiBaseUrl } from "@/lib/api/client";

export default function PerformancePage() {
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [benchData, setBenchData] = useState<any | null>(null);

  // Configuration state
  const [batchSize, setBatchSize] = useState<string>("100");
  const [elementsPerSim, setElementsPerSim] = useState<string>("40");
  const [seed, setSeed] = useState<string>("42");
  const [timedReps, setTimedReps] = useState<string>("5");

  const runBenchmark = async () => {
    setLoading(true);
    setError(null);
    try {
      const base = getApiBaseUrl();
      const payload = {
        batch_size: parseInt(batchSize, 10),
        elements_per_sim: parseInt(elementsPerSim, 10),
        seed: parseInt(seed, 10),
        timed_reps: parseInt(timedReps, 10),
        worker_counts: [1, 2, 3, 4],
        warmup_reps: 1,
      };

      const res = await fetch(`${base}/api/v1/performance/run`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });

      const data = await res.json();
      if (!res.ok) {
        throw new Error(data.error || "Benchmark execution failed.");
      }

      setBenchData(data);
    } catch (err: any) {
      setError(err?.message || "Failed to execute multicore benchmark.");
    } finally {
      setLoading(false);
    }
  };

  const downloadCsv = () => {
    if (!benchData || !benchData.csv_export) return;
    const blob = new Blob([benchData.csv_export], { type: "text/csv;charset=utf-8;" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.setAttribute("download", "benchmark_results.csv");
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-4 pb-4 border-b border-gray-800">
        <div>
          <div className="flex items-center gap-2">
            <h1 className="text-2xl font-bold text-white tracking-tight">Multicore Execution Benchmark</h1>
            <span className="text-xs font-mono px-2 py-0.5 rounded bg-emerald-900/60 text-emerald-300 border border-emerald-700/50">Task D</span>
          </div>
          <p className="text-xs text-gray-400 mt-1">
            Serial vs parallel batch simulation benchmark across 1–4 CPU workers with agreement verification (ΔT ≤ 10⁻⁸ °C).
          </p>
        </div>

        {benchData && (
          <button
            onClick={downloadCsv}
            className="flex items-center gap-2 px-3.5 py-2 rounded-lg bg-gray-900 hover:bg-gray-800 border border-gray-700 text-xs font-medium text-emerald-300 transition-colors shadow"
          >
            <Download className="w-4 h-4" />
            <span>Download CSV (benchmark_results.csv)</span>
          </button>
        )}
      </div>

      {/* Main Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Column: Configuration & Hardware Manifest */}
        <div className="space-y-5">
          {/* Card 1: Benchmark Configuration */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
            <div className="flex items-center justify-between">
              <h2 className="text-xs font-bold text-white flex items-center gap-2">
                <Cpu className="w-4 h-4 text-emerald-400" />
                <span>Benchmark Configuration</span>
              </h2>
              <span className="text-[10px] font-mono text-gray-400 bg-gray-900 px-2 py-0.5 rounded border border-gray-800">
                1–4 Workers
              </span>
            </div>

            <div className="space-y-3 text-xs">
              <div>
                <label className="text-gray-400 block mb-1">Batch Case Count</label>
                <input 
                  type="number" 
                  value={batchSize} 
                  onChange={(e) => setBatchSize(e.target.value)}
                  className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-emerald-500" 
                />
              </div>
              <div>
                <label className="text-gray-400 block mb-1">Mesh Elements per Case (N)</label>
                <input 
                  type="number" 
                  value={elementsPerSim} 
                  onChange={(e) => setElementsPerSim(e.target.value)}
                  className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-emerald-500" 
                />
              </div>
              <div>
                <label className="text-gray-400 block mb-1">Deterministic Seed (RNG)</label>
                <input 
                  type="number" 
                  value={seed} 
                  onChange={(e) => setSeed(e.target.value)}
                  className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-emerald-500" 
                />
              </div>
              <div>
                <label className="text-gray-400 block mb-1">Measured Repetitions</label>
                <input 
                  type="number" 
                  value={timedReps} 
                  onChange={(e) => setTimedReps(e.target.value)}
                  className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-emerald-500" 
                />
              </div>
            </div>

            <button 
              onClick={runBenchmark}
              disabled={loading}
              className="w-full py-2.5 px-4 rounded-lg bg-emerald-600 hover:bg-emerald-500 disabled:bg-emerald-800 disabled:opacity-60 font-semibold text-xs text-white shadow-lg shadow-emerald-600/20 flex items-center justify-center gap-2 transition-all cursor-pointer"
            >
              {loading ? (
                <>
                  <Loader2 className="w-4 h-4 animate-spin" />
                  <span>Benchmarking (1, 2, 3, 4 Workers)...</span>
                </>
              ) : (
                <>
                  <Play className="w-4 h-4 fill-current" />
                  <span>Run Benchmark Suite (1–4 Workers)</span>
                </>
              )}
            </button>

            {error && (
              <div className="p-3 rounded-lg bg-red-950/60 border border-red-800/80 text-xs text-red-300 flex items-start gap-2">
                <AlertTriangle className="w-4 h-4 text-red-400 shrink-0 mt-0.5" />
                <div className="leading-relaxed">{error}</div>
              </div>
            )}
          </div>

          {/* Card 2: Host Hardware Manifest */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-3">
            <h3 className="text-xs font-bold text-white flex items-center gap-2">
              <HardDrive className="w-4 h-4 text-emerald-400" />
              <span>Host Hardware Manifest</span>
            </h3>

            <div className="text-xs font-mono space-y-1.5 bg-gray-900/80 p-3 rounded-lg border border-gray-800 text-gray-300">
              <div className="flex justify-between">
                <span className="text-gray-400">Logical Cores:</span>
                <span className="text-emerald-400 font-bold">
                  {benchData ? benchData.logical_cores : "Auto-detected"}
                </span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Worker Counts:</span>
                <span className="text-white font-bold">1, 2, 3, 4 threads</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Timing Method:</span>
                <span className="text-gray-200">std::chrono high_res</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Filtering:</span>
                <span className="text-gray-200">Median of 5 runs</span>
              </div>
              {benchData && benchData.hardware_model && (
                <div className="pt-2 mt-2 border-t border-gray-800 text-[10px] text-gray-400 truncate">
                  Model: {benchData.hardware_model}
                </div>
              )}
            </div>
          </div>
        </div>

        {/* Right Column: Speedup Plot & Verification Table */}
        <div className="lg:col-span-2 space-y-6">
          {/* Speedup & Efficiency Curve */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 flex flex-col justify-between">
            <div className="flex items-center justify-between pb-3 border-b border-gray-800 mb-4">
              <div className="text-xs font-bold text-white flex items-center gap-2">
                <TrendingUp className="w-4 h-4 text-emerald-400" />
                <span>Speedup Sp = T1 / Tp vs Linear Ideal</span>
              </div>
              <span className="text-[11px] font-mono text-gray-400">
                1 Warmup + 5 Timed Repetitions
              </span>
            </div>

            {benchData && benchData.worker_results ? (
              <SpeedupChart results={benchData.worker_results} />
            ) : (
              <div className="flex flex-col items-center justify-center py-16 text-center">
                <Cpu className="w-12 h-12 text-gray-700 mb-3 animate-pulse" />
                <div className="text-xs font-medium text-gray-300">Benchmark Not Yet Executed</div>
                <p className="text-[11px] text-gray-500 max-w-sm mt-1">
                  Click &quot;Run Benchmark Suite&quot; to execute 100 batch rod simulations across 1, 2, 3, and 4 worker threads.
                </p>
                <button
                  onClick={runBenchmark}
                  disabled={loading}
                  className="mt-4 px-4 py-2 rounded-lg bg-emerald-600 hover:bg-emerald-500 text-xs font-medium text-white shadow-md shadow-emerald-500/20 transition-all flex items-center gap-1.5 cursor-pointer"
                >
                  <Play className="w-3.5 h-3.5 fill-current" />
                  <span>Run Preset Benchmark</span>
                </button>
              </div>
            )}
          </div>

          {/* Verification Table */}
          {benchData && benchData.worker_results && (
            <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800">
              <BenchmarkTable 
                results={benchData.worker_results} 
                agreementVerified={benchData.agreement_verified} 
              />
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
