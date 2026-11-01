"use client";

import React, { useState } from "react";
import { 
  Flame, 
  Play, 
  Download, 
  BarChart2, 
  CheckCircle2, 
  AlertTriangle, 
  Loader2,
  Sparkles,
  Info
} from "lucide-react";
import { ThermalChart } from "@/components/thermal/ThermalChart";
import { ConvergenceTable } from "@/components/thermal/ConvergenceTable";
import { getApiBaseUrl } from "@/lib/api/client";

export default function ThermalPage() {
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [studyResult, setStudyResult] = useState<any | null>(null);

  // Form input parameters (matching assignment specifications)
  const [lengthL, setLengthL] = useState<string>("1.0");
  const [areaA, setAreaA] = useState<string>("1e-4");
  const [condK, setCondK] = useState<string>("10.0");
  const [heatQ, setHeatQ] = useState<string>("10000.0");
  const [tempLeft, setTempLeft] = useState<string>("100.0");
  const [tempRight, setTempRight] = useState<string>("20.0");

  const runSimulation = async () => {
    setLoading(true);
    setError(null);
    try {
      const base = getApiBaseUrl();
      const payload = {
        L: parseFloat(lengthL),
        A: parseFloat(areaA),
        k: parseFloat(condK),
        q: parseFloat(heatQ),
        T_left: parseFloat(tempLeft),
        T_right: parseFloat(tempRight),
        mesh_sizes: [10, 20, 40, 80],
      };

      const res = await fetch(`${base}/api/v1/thermal/solve`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });

      const data = await res.json();
      if (!res.ok) {
        throw new Error(data.error || "Simulation failed.");
      }

      setStudyResult(data);
    } catch (err: any) {
      setError(err?.message || "Failed to execute thermal FEA study.");
    } finally {
      setLoading(false);
    }
  };

  const downloadCsv = () => {
    if (!studyResult || !studyResult.csv_export) return;
    const blob = new Blob([studyResult.csv_export], { type: "text/csv;charset=utf-8;" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.setAttribute("download", "thermal_convergence.csv");
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
            <h1 className="text-2xl font-bold text-white tracking-tight">1D Steady-State Thermal FEA Solver</h1>
            <span className="text-xs font-mono px-2 py-0.5 rounded bg-amber-900/60 text-amber-300 border border-amber-700/50">Task C</span>
          </div>
          <p className="text-xs text-gray-400 mt-1">
            Uniform 2-node linear element finite-element solver with consistent load vector qA and exact Dirichlet elimination.
          </p>
        </div>

        {studyResult && (
          <button
            onClick={downloadCsv}
            className="flex items-center gap-2 px-3.5 py-2 rounded-lg bg-gray-900 hover:bg-gray-800 border border-gray-700 text-xs font-medium text-amber-300 transition-colors shadow"
          >
            <Download className="w-4 h-4" />
            <span>Download CSV (thermal_convergence.csv)</span>
          </button>
        )}
      </div>

      {/* Main Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left Column: Input Parameters */}
        <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4 h-fit">
          <div className="flex items-center justify-between">
            <h2 className="text-sm font-bold text-white flex items-center gap-2">
              <Flame className="w-4 h-4 text-amber-400" />
              <span>Assignment Input Parameters</span>
            </h2>
            <span className="text-[10px] font-mono text-gray-400 bg-gray-900 px-2 py-0.5 rounded border border-gray-800">
              Task C Preset
            </span>
          </div>

          <div className="space-y-3 text-xs">
            <div>
              <label className="text-gray-400 block mb-1">Rod Length L (m)</label>
              <input 
                type="text" 
                value={lengthL} 
                onChange={(e) => setLengthL(e.target.value)}
                className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-amber-500" 
              />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Cross-Sectional Area A (m²)</label>
              <input 
                type="text" 
                value={areaA} 
                onChange={(e) => setAreaA(e.target.value)}
                className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-amber-500" 
              />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Thermal Conductivity k (W/(m·K))</label>
              <input 
                type="text" 
                value={condK} 
                onChange={(e) => setCondK(e.target.value)}
                className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-amber-500" 
              />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Heat Generation q (W/m³)</label>
              <input 
                type="text" 
                value={heatQ} 
                onChange={(e) => setHeatQ(e.target.value)}
                className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-amber-500" 
              />
            </div>
            <div className="grid grid-cols-2 gap-2">
              <div>
                <label className="text-gray-400 block mb-1">T(0) Left (°C)</label>
                <input 
                  type="text" 
                  value={tempLeft} 
                  onChange={(e) => setTempLeft(e.target.value)}
                  className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-amber-300 focus:outline-none focus:border-amber-500" 
                />
              </div>
              <div>
                <label className="text-gray-400 block mb-1">T(L) Right (°C)</label>
                <input 
                  type="text" 
                  value={tempRight} 
                  onChange={(e) => setTempRight(e.target.value)}
                  className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-blue-300 focus:outline-none focus:border-amber-500" 
                />
              </div>
            </div>
          </div>

          <button 
            onClick={runSimulation}
            disabled={loading}
            className="w-full py-2.5 px-4 rounded-lg bg-amber-600 hover:bg-amber-500 disabled:bg-amber-800 disabled:opacity-60 font-semibold text-xs text-white shadow-lg shadow-amber-600/20 flex items-center justify-center gap-2 transition-all cursor-pointer"
          >
            {loading ? (
              <>
                <Loader2 className="w-4 h-4 animate-spin" />
                <span>Solving C++ FEA (N = 10, 20, 40, 80)...</span>
              </>
            ) : (
              <>
                <Play className="w-4 h-4 fill-current" />
                <span>Execute Mesh Study (N = 10, 20, 40, 80)</span>
              </>
            )}
          </button>

          {error && (
            <div className="p-3 rounded-lg bg-red-950/60 border border-red-800/80 text-xs text-red-300 flex items-start gap-2">
              <AlertTriangle className="w-4 h-4 text-red-400 shrink-0 mt-0.5" />
              <div className="leading-relaxed">{error}</div>
            </div>
          )}

          {/* Mathematical Formulation Reference Card */}
          <div className="p-3.5 rounded-lg bg-gray-900/80 border border-gray-800 text-[11px] space-y-2 text-gray-300">
            <div className="font-bold text-gray-200 flex items-center gap-1.5">
              <Info className="w-3.5 h-3.5 text-amber-400" />
              <span>Governing Differential Equation</span>
            </div>
            <div className="font-mono bg-black/40 p-2 rounded text-amber-300/90 text-[10px] space-y-1">
              <div>-d/dx (k A dT/dx) = q A</div>
              <div>BCs: T(0) = 100°C, T(1) = 20°C</div>
              <div className="text-gray-400 pt-1 border-t border-gray-800">
                Exact: T(x) = 100 - 80x + 500x(1-x)
              </div>
            </div>
            <div className="text-[10px] text-gray-400 leading-relaxed">
              Stiffness: <span className="font-mono text-gray-300">(kA/h)[[1,-1],[-1,1]]</span><br />
              Consistent Load: <span className="font-mono text-gray-300">(qAh/2)[1,1]ᵀ</span>
            </div>
          </div>
        </div>

        {/* Right Column: Visualization & Convergence Table */}
        <div className="lg:col-span-2 space-y-6">
          {/* Temperature Distribution Plot */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 flex flex-col justify-between">
            <div className="flex items-center justify-between pb-3 border-b border-gray-800 mb-4">
              <div className="text-xs font-bold text-white flex items-center gap-2">
                <BarChart2 className="w-4 h-4 text-amber-400" />
                <span>Temperature Distribution T(x) vs Analytical Solution</span>
              </div>
              <span className="text-[11px] font-mono text-gray-400">
                Analytical exact overlay (dashed)
              </span>
            </div>

            {studyResult && studyResult.solutions ? (
              <ThermalChart solutions={studyResult.solutions} />
            ) : (
              <div className="flex flex-col items-center justify-center py-16 text-center">
                <Flame className="w-12 h-12 text-gray-700 mb-3 animate-pulse" />
                <div className="text-xs font-medium text-gray-300">Simulation Not Yet Run</div>
                <p className="text-[11px] text-gray-500 max-w-sm mt-1">
                  Click &quot;Execute Mesh Study&quot; to solve the 1D FEA problem across 4 mesh densities (N = 10, 20, 40, 80) and compare with analytical truth.
                </p>
                <button
                  onClick={runSimulation}
                  disabled={loading}
                  className="mt-4 px-4 py-2 rounded-lg bg-amber-600 hover:bg-amber-500 text-xs font-medium text-white shadow-md shadow-amber-500/20 transition-all flex items-center gap-1.5 cursor-pointer"
                >
                  <Play className="w-3.5 h-3.5 fill-current" />
                  <span>Run Preset Mesh Study</span>
                </button>
              </div>
            )}
          </div>

          {/* Convergence Table */}
          {studyResult && studyResult.solutions && (
            <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800">
              <ConvergenceTable solutions={studyResult.solutions} />
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
