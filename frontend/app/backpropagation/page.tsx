"use client";

import React, { useState } from "react";
import { 
  Binary, 
  Play, 
  Download, 
  CheckCircle2, 
  TrendingDown, 
  AlertTriangle, 
  Loader2,
  FileText,
  Info,
  Sparkles
} from "lucide-react";
import { LossChart } from "@/components/neural/LossChart";
import { GradientTable } from "@/components/neural/GradientTable";
import { getApiBaseUrl } from "@/lib/api/client";

export default function BackpropagationPage() {
  const [loading, setLoading] = useState<boolean>(false);
  const [error, setError] = useState<string | null>(null);
  const [trainData, setTrainData] = useState<any | null>(null);

  // Initial parameters state
  const [inputX, setInputX] = useState<string>("1.0");
  const [targetY, setTargetY] = useState<string>("2.0");
  const [weightW, setWeightW] = useState<string>("0.5");
  const [biasB, setBiasB] = useState<string>("0.0");
  const [weightV, setWeightV] = useState<string>("1.0");
  const [biasC, setBiasC] = useState<string>("0.0");
  const [learningRate, setLearningRate] = useState<string>("0.1");
  const [steps, setSteps] = useState<string>("100");

  const runTraining = async () => {
    setLoading(true);
    setError(null);
    try {
      const base = getApiBaseUrl();
      const payload = {
        x: parseFloat(inputX),
        y: parseFloat(targetY),
        w: parseFloat(weightW),
        b: parseFloat(biasB),
        v: parseFloat(weightV),
        c: parseFloat(biasC),
        lr: parseFloat(learningRate),
        steps: parseInt(steps, 10),
        epsilon: 1.0e-6,
      };

      const res = await fetch(`${base}/api/v1/neural/train`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });

      const data = await res.json();
      if (!res.ok) {
        throw new Error(data.error || "Training failed.");
      }

      setTrainData(data);
    } catch (err: any) {
      setError(err?.message || "Failed to execute backpropagation training.");
    } finally {
      setLoading(false);
    }
  };

  const downloadCsv = () => {
    if (!trainData || !trainData.csv_export) return;
    const blob = new Blob([trainData.csv_export], { type: "text/csv;charset=utf-8;" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.setAttribute("download", "backpropagation_training.csv");
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
            <h1 className="text-2xl font-bold text-white tracking-tight">Neural Backpropagation & Gradient Verification</h1>
            <span className="text-xs font-mono px-2 py-0.5 rounded bg-pink-900/60 text-pink-300 border border-pink-700/50">Task F</span>
          </div>
          <p className="text-xs text-gray-400 mt-1">
            Handcrafted 1-sample regression training in pure C++ without auto-diff; central finite differences gradient validation (error ≤ 10⁻⁵).
          </p>
        </div>

        {trainData && (
          <button
            onClick={downloadCsv}
            className="flex items-center gap-2 px-3.5 py-2 rounded-lg bg-gray-900 hover:bg-gray-800 border border-gray-700 text-xs font-medium text-pink-300 transition-colors shadow"
          >
            <Download className="w-4 h-4" />
            <span>Download CSV (backpropagation_training.csv)</span>
          </button>
        )}
      </div>

      {/* Main Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left: Model Parameters & Equations */}
        <div className="space-y-5">
          {/* Card 1: Configuration Form */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
            <div className="flex items-center justify-between">
              <h2 className="text-xs font-bold text-white flex items-center gap-2">
                <Binary className="w-4 h-4 text-pink-400" />
                <span>Model Architecture & Parameters</span>
              </h2>
              <span className="text-[10px] font-mono text-gray-400 bg-gray-900 px-2 py-0.5 rounded border border-gray-800">
                1 Sample Regression
              </span>
            </div>

            <div className="space-y-3 text-xs">
              <div className="grid grid-cols-2 gap-2">
                <div>
                  <label className="text-gray-400 block mb-1">Input (x)</label>
                  <input 
                    type="text" 
                    value={inputX} 
                    onChange={e => setInputX(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-pink-500" 
                  />
                </div>
                <div>
                  <label className="text-gray-400 block mb-1">Target (y)</label>
                  <input 
                    type="text" 
                    value={targetY} 
                    onChange={e => setTargetY(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-emerald-300 focus:outline-none focus:border-pink-500" 
                  />
                </div>
              </div>

              <div className="grid grid-cols-2 gap-2">
                <div>
                  <label className="text-gray-400 block mb-1">Weight w₀</label>
                  <input 
                    type="text" 
                    value={weightW} 
                    onChange={e => setWeightW(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-pink-300 focus:outline-none focus:border-pink-500" 
                  />
                </div>
                <div>
                  <label className="text-gray-400 block mb-1">Bias b₀</label>
                  <input 
                    type="text" 
                    value={biasB} 
                    onChange={e => setBiasB(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-pink-500" 
                  />
                </div>
              </div>

              <div className="grid grid-cols-2 gap-2">
                <div>
                  <label className="text-gray-400 block mb-1">Weight v₀</label>
                  <input 
                    type="text" 
                    value={weightV} 
                    onChange={e => setWeightV(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-pink-300 focus:outline-none focus:border-pink-500" 
                  />
                </div>
                <div>
                  <label className="text-gray-400 block mb-1">Bias c₀</label>
                  <input 
                    type="text" 
                    value={biasC} 
                    onChange={e => setBiasC(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-pink-500" 
                  />
                </div>
              </div>

              <div className="grid grid-cols-2 gap-2">
                <div>
                  <label className="text-gray-400 block mb-1">Learning Rate (η)</label>
                  <input 
                    type="text" 
                    value={learningRate} 
                    onChange={e => setLearningRate(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-pink-500" 
                  />
                </div>
                <div>
                  <label className="text-gray-400 block mb-1">Steps (epochs)</label>
                  <input 
                    type="number" 
                    value={steps} 
                    onChange={e => setSteps(e.target.value)}
                    className="w-full bg-gray-900 border border-gray-700 rounded px-3 py-1.5 font-mono text-gray-200 focus:outline-none focus:border-pink-500" 
                  />
                </div>
              </div>
            </div>

            <button 
              onClick={runTraining}
              disabled={loading}
              className="w-full py-2.5 px-4 rounded-lg bg-pink-600 hover:bg-pink-500 disabled:bg-pink-800 disabled:opacity-60 font-semibold text-xs text-white shadow-lg shadow-pink-600/20 flex items-center justify-center gap-2 transition-all cursor-pointer"
            >
              {loading ? (
                <>
                  <Loader2 className="w-4 h-4 animate-spin" />
                  <span>Executing 100-Step Training...</span>
                </>
              ) : (
                <>
                  <Play className="w-4 h-4 fill-current" />
                  <span>Run Gradient Check & 100 Steps</span>
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

          {/* Card 2: Mathematical Derivation Reference */}
          <div className="p-4 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-2 text-xs text-gray-300">
            <div className="font-bold text-white flex items-center gap-1.5 text-xs">
              <FileText className="w-3.5 h-3.5 text-pink-400" />
              <span>Forward & Backward Formulas</span>
            </div>
            <div className="font-mono bg-black/40 p-2.5 rounded text-[10px] space-y-1 text-pink-300/90">
              <div>Forward:</div>
              <div>• z = w·x + b</div>
              <div>• h = tanh(z)</div>
              <div>• ŷ = v·h + c</div>
              <div>• J = 0.5·(ŷ - y)²</div>
              <div className="pt-1 text-gray-400 border-t border-gray-800">Backward:</div>
              <div>• δ = ŷ - y</div>
              <div>• ∂J/∂c = δ</div>
              <div>• ∂J/∂v = δ·h</div>
              <div>• ∂J/∂w = δ·v·(1-h²)·x</div>
              <div>• ∂J/∂b = δ·v·(1-h²)</div>
            </div>
            <div className="text-[10px] text-gray-400 pt-1">
              Doc: <span className="font-mono text-gray-300">docs/reference/handwritten-derivation-guide.md</span>
            </div>
          </div>
        </div>

        {/* Right: Validation & Loss Curve */}
        <div className="lg:col-span-2 space-y-6">
          {/* Loss Curve */}
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 flex flex-col justify-between">
            <div className="flex items-center justify-between pb-3 border-b border-gray-800 mb-4">
              <div className="text-xs font-bold text-white flex items-center gap-2">
                <TrendingDown className="w-4 h-4 text-pink-400" />
                <span>Training Loss Convergence J(θ) Across 100 Steps</span>
              </div>
              {trainData && (
                <span className="text-[11px] font-mono text-emerald-400">
                  Initial Loss: {trainData.history[0].loss.toFixed(4)} → Final: {trainData.history[trainData.history.length - 1].loss.toExponential(2)}
                </span>
              )}
            </div>

            {trainData && trainData.history ? (
              <LossChart history={trainData.history} />
            ) : (
              <div className="flex flex-col items-center justify-center py-16 text-center">
                <Binary className="w-12 h-12 text-gray-700 mb-3 animate-pulse" />
                <div className="text-xs font-medium text-gray-300">Training Not Yet Run</div>
                <p className="text-[11px] text-gray-500 max-w-sm mt-1">
                  Click &quot;Run Gradient Check & 100 Steps&quot; to execute handcrafted C++ backpropagation and compare with central finite differences.
                </p>
                <button
                  onClick={runTraining}
                  disabled={loading}
                  className="mt-4 px-4 py-2 rounded-lg bg-pink-600 hover:bg-pink-500 text-xs font-medium text-white shadow-md shadow-pink-500/20 transition-all flex items-center gap-1.5 cursor-pointer"
                >
                  <Play className="w-3.5 h-3.5 fill-current" />
                  <span>Run Preset Training</span>
                </button>
              </div>
            )}
          </div>

          {/* Gradient Verification Table */}
          {trainData && trainData.grad_check && (
            <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800">
              <GradientTable 
                entries={trainData.grad_check.entries}
                epsilon={trainData.grad_check.epsilon}
                tolerance={trainData.grad_check.tolerance}
              />
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
