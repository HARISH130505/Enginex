"use client";

import React from "react";
import { Cpu, Play, CheckCircle2, TrendingUp, BarChart } from "lucide-react";

export default function PerformancePage() {
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
      </div>

      {/* Main Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left: Configuration */}
        <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
          <h2 className="text-sm font-bold text-white flex items-center gap-2">
            <Cpu className="w-4 h-4 text-emerald-400" />
            <span>Benchmark Configuration</span>
          </h2>

          <div className="space-y-3 text-xs">
            <div>
              <label className="text-gray-400 block mb-1">Batch Case Count</label>
              <input type="number" readOnly value="100" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Mesh Elements per Case (N)</label>
              <input type="number" readOnly value="80" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Deterministic Seed</label>
              <input type="number" readOnly value="42" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Repetitions per Worker Count</label>
              <input type="text" readOnly value="1 Warmup + 5 Measured Runs" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
          </div>

          <button className="w-full py-2.5 px-4 rounded-lg bg-emerald-600 hover:bg-emerald-500 font-semibold text-xs text-white shadow-lg shadow-emerald-600/20 flex items-center justify-center gap-2 transition-all">
            <Play className="w-4 h-4 fill-current" />
            <span>Run Benchmark Suite (1, 2, 3, 4 Workers)</span>
          </button>
        </div>

        {/* Right: Results Placeholder */}
        <div className="lg:col-span-2 space-y-6">
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 min-h-[300px] flex flex-col justify-between">
            <div className="flex items-center justify-between pb-3 border-b border-gray-800">
              <div className="text-xs font-bold text-white flex items-center gap-2">
                <TrendingUp className="w-4 h-4 text-emerald-400" />
                <span>Speedup (Sp = T1/Tp) and Efficiency (Ep = Sp/p)</span>
              </div>
              <span className="text-[11px] font-mono text-gray-400">High-Resolution Monotonic Timings</span>
            </div>

            <div className="flex-1 flex flex-col items-center justify-center py-12 text-center">
              <Cpu className="w-12 h-12 text-gray-700 mb-2 animate-pulse" />
              <div className="text-xs font-medium text-gray-400">Ready for Phase G4 benchmark execution</div>
              <p className="text-[11px] text-gray-500 max-w-sm mt-1">
                Will report 5 repetitions, median execution times, speedup curves, and verify strict serial/parallel numerical agreement.
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

