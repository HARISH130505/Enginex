"use client";

import React from "react";
import { Table, ShieldCheck } from "lucide-react";

interface WorkerResult {
  workers: number;
  rep_durations_ms: number[];
  median_ms: number;
  speedup: number;
  efficiency: number;
  max_temp_diff_vs_serial: number;
}

interface BenchmarkTableProps {
  results: WorkerResult[];
  agreementVerified: boolean;
}

export function BenchmarkTable({ results, agreementVerified }: BenchmarkTableProps) {
  if (!results || results.length === 0) return null;

  return (
    <div className="space-y-3">
      <div className="flex items-center justify-between">
        <h3 className="text-xs font-bold text-white flex items-center gap-2">
          <Table className="w-4 h-4 text-emerald-400" />
          <span>Execution Timings, Speedup, Efficiency & Agreement Verification</span>
        </h3>
        {agreementVerified && (
          <span className="text-[11px] font-mono text-emerald-400 flex items-center gap-1 bg-emerald-950/60 px-2 py-0.5 rounded border border-emerald-800/60">
            <ShieldCheck className="w-3.5 h-3.5" />
            <span>ΔT ≤ 10⁻⁸ °C Verified</span>
          </span>
        )}
      </div>

      <div className="overflow-x-auto rounded-lg border border-gray-800 bg-gray-900/60">
        <table className="w-full text-left text-xs font-mono">
          <thead className="bg-gray-800/80 text-gray-300 text-[11px] uppercase tracking-wider border-b border-gray-700">
            <tr>
              <th className="px-3 py-2.5">Workers (p)</th>
              <th className="px-3 py-2.5">Rep 1 (ms)</th>
              <th className="px-3 py-2.5">Rep 2 (ms)</th>
              <th className="px-3 py-2.5">Rep 3 (ms)</th>
              <th className="px-3 py-2.5">Rep 4 (ms)</th>
              <th className="px-3 py-2.5">Rep 5 (ms)</th>
              <th className="px-3 py-2.5">Median (ms)</th>
              <th className="px-3 py-2.5">Speedup (Sp)</th>
              <th className="px-3 py-2.5">Efficiency (Ep)</th>
              <th className="px-3 py-2.5">Max Discrepancy</th>
            </tr>
          </thead>
          <tbody className="divide-y divide-gray-800 text-gray-300">
            {results.map((wr) => {
              const effPct = (wr.efficiency * 100).toFixed(1);
              return (
                <tr key={wr.workers} className="hover:bg-gray-800/40 transition-colors">
                  <td className="px-3 py-2 text-white font-bold">{wr.workers}</td>
                  {wr.rep_durations_ms.slice(0, 5).map((d, i) => (
                    <td key={i} className="px-3 py-2 text-gray-400">
                      {d.toFixed(1)}
                    </td>
                  ))}
                  {Array.from({ length: Math.max(0, 5 - wr.rep_durations_ms.length) }).map((_, i) => (
                    <td key={`empty-${i}`} className="px-3 py-2 text-gray-600">—</td>
                  ))}
                  <td className="px-3 py-2 text-emerald-300 font-bold">{wr.median_ms.toFixed(1)}</td>
                  <td className="px-3 py-2 text-emerald-400 font-bold">{wr.speedup.toFixed(2)}×</td>
                  <td className="px-3 py-2">
                    <span className="inline-flex items-center px-1.5 py-0.5 rounded text-[10px] font-bold bg-gray-800 text-gray-200 border border-gray-700">
                      {effPct}%
                    </span>
                  </td>
                  <td className="px-3 py-2">
                    {wr.workers === 1 ? (
                      <span className="text-gray-500 font-sans text-[11px]">Baseline</span>
                    ) : (
                      <span className="text-emerald-400 font-mono text-[10px]">
                        {wr.max_temp_diff_vs_serial.toExponential(2)} °C
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
        <span>Warmup: 1 run excluded. Median calculation prevents OS scheduling jitter.</span>
        <span className="font-mono text-emerald-400">Tolerance: |T_serial - T_parallel| ≤ 1.00e-08 °C</span>
      </div>
    </div>
  );
}
