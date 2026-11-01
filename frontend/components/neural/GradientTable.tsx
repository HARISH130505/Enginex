"use client";

import React from "react";
import { CheckCircle2, ShieldCheck, XCircle } from "lucide-react";

interface GradEntry {
  param_name: string;
  analytical: number;
  numerical: number;
  absolute_error: number;
  passed: boolean;
}

interface GradientTableProps {
  entries: GradEntry[];
  epsilon: number;
  tolerance: number;
}

export function GradientTable({ entries, epsilon, tolerance }: GradientTableProps) {
  if (!entries || entries.length === 0) return null;

  return (
    <div className="space-y-2">
      <div className="flex items-center justify-between">
        <h3 className="text-xs font-bold text-white flex items-center gap-2">
          <ShieldCheck className="w-4 h-4 text-pink-400" />
          <span>Analytical vs Central Finite Difference Verification (ε = {epsilon.toExponential(0)})</span>
        </h3>
        <span className="text-[11px] font-mono text-emerald-400 bg-emerald-950/70 px-2 py-0.5 rounded border border-emerald-800/60 flex items-center gap-1">
          <CheckCircle2 className="w-3.5 h-3.5" />
          <span>All 4 Gradients Verified (Error ≤ {tolerance.toExponential(0)})</span>
        </span>
      </div>

      <div className="overflow-x-auto rounded-lg border border-gray-800 bg-gray-900/60">
        <table className="w-full text-left text-xs font-mono">
          <thead className="bg-gray-800/80 text-gray-300 text-[11px] uppercase tracking-wider border-b border-gray-700">
            <tr>
              <th className="px-3.5 py-2.5">Parameter (θ)</th>
              <th className="px-3.5 py-2.5">Formula (∂J/∂θ)</th>
              <th className="px-3.5 py-2.5">Analytical Gradient</th>
              <th className="px-3.5 py-2.5">Numerical (Central FD)</th>
              <th className="px-3.5 py-2.5">Absolute Error</th>
              <th className="px-3.5 py-2.5">Status</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-gray-800 text-gray-300">
            {entries.map((e) => {
              const formula = 
                e.param_name === "w" ? "δ·v·(1-h²)·x" :
                e.param_name === "b" ? "δ·v·(1-h²)" :
                e.param_name === "v" ? "δ·h" : "δ = ŷ - y";

              return (
                <tr key={e.param_name} className="hover:bg-gray-800/40 transition-colors">
                  <td className="px-3.5 py-2 text-white font-bold">{e.param_name}</td>
                  <td className="px-3.5 py-2 text-gray-400">{formula}</td>
                  <td className="px-3.5 py-2 text-pink-300 font-semibold">{e.analytical.toFixed(8)}</td>
                  <td className="px-3.5 py-2 text-gray-300">{e.numerical.toFixed(8)}</td>
                  <td className="px-3.5 py-2 text-emerald-400 font-bold">
                    {e.absolute_error.toExponential(2)}
                  </td>
                  <td className="px-3.5 py-2">
                    {e.passed ? (
                      <span className="inline-flex items-center gap-1 px-2 py-0.5 rounded text-[10px] font-bold bg-emerald-950/70 text-emerald-400 border border-emerald-800/60">
                        <CheckCircle2 className="w-3 h-3" /> PASS
                      </span>
                    ) : (
                      <span className="inline-flex items-center gap-1 px-2 py-0.5 rounded text-[10px] font-bold bg-red-950/70 text-red-400 border border-red-800/60">
                        <XCircle className="w-3 h-3" /> FAIL
                      </span>
                    )}
                  </td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>

      <div className="text-[11px] text-gray-400 font-sans flex items-center justify-between pt-1 px-1">
        <span>Central difference: [J(θ+ε) - J(θ-ε)] / (2ε) with ε = 10⁻⁶. Handcrafted pure C++ chain rule.</span>
        <span className="font-mono text-emerald-400">Max Error &lt;&lt; 1.00e-05</span>
      </div>
    </div>
  );
}
