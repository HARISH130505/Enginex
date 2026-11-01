"use client";

import React from "react";
import { Flame, Play, Download, BarChart2, CheckCircle2 } from "lucide-react";

export default function ThermalPage() {
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
      </div>

      {/* Main Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left: Input Parameters */}
        <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
          <h2 className="text-sm font-bold text-white flex items-center gap-2">
            <Flame className="w-4 h-4 text-amber-400" />
            <span>Assignment Input Parameters</span>
          </h2>

          <div className="space-y-3 text-xs">
            <div>
              <label className="text-gray-400 block mb-1">Rod Length L (m)</label>
              <input type="text" readOnly value="1.0" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Cross-Sectional Area A (m²)</label>
              <input type="text" readOnly value="1e-4" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Thermal Conductivity k (W/(m·K))</label>
              <input type="text" readOnly value="10.0" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
            <div>
              <label className="text-gray-400 block mb-1">Heat Generation q (W/m³)</label>
              <input type="text" readOnly value="10000.0" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200" />
            </div>
            <div className="grid grid-cols-2 gap-2">
              <div>
                <label className="text-gray-400 block mb-1">T(0) Left (°C)</label>
                <input type="text" readOnly value="100.0" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-amber-300" />
              </div>
              <div>
                <label className="text-gray-400 block mb-1">T(L) Right (°C)</label>
                <input type="text" readOnly value="20.0" className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-blue-300" />
              </div>
            </div>
          </div>

          <button className="w-full py-2.5 px-4 rounded-lg bg-amber-600 hover:bg-amber-500 font-semibold text-xs text-white shadow-lg shadow-amber-600/20 flex items-center justify-center gap-2 transition-all">
            <Play className="w-4 h-4 fill-current" />
            <span>Execute Mesh Study (N = 10, 20, 40, 80)</span>
          </button>
        </div>

        {/* Right: Plots & Verification Table */}
        <div className="lg:col-span-2 space-y-6">
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 min-h-[300px] flex flex-col justify-between">
            <div className="flex items-center justify-between pb-3 border-b border-gray-800">
              <div className="text-xs font-bold text-white flex items-center gap-2">
                <BarChart2 className="w-4 h-4 text-amber-400" />
                <span>Temperature Distribution T(x) vs Exact Analytical Solution</span>
              </div>
              <span className="text-[11px] font-mono text-gray-400">T_exact(x) = 100 - 80x/L + [q/(2k)]x(L-x)</span>
            </div>

            <div className="flex-1 flex flex-col items-center justify-center py-12 text-center">
              <Flame className="w-12 h-12 text-gray-700 mb-2 animate-pulse" />
              <div className="text-xs font-medium text-gray-400">Ready to simulate in Phase G3</div>
              <p className="text-[11px] text-gray-500 max-w-sm mt-1">
                Will render interactive multi-mesh comparison curves and midpoint error convergence rates.
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

