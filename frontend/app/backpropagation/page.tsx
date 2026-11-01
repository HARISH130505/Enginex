"use client";

import React from "react";
import { Binary, Play, CheckCircle2, FileText, TrendingDown } from "lucide-react";

export default function BackpropagationPage() {
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
      </div>

      {/* Main Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Left: Mathematical Model & Initial Parameters */}
        <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 space-y-4">
          <h2 className="text-sm font-bold text-white flex items-center gap-2">
            <Binary className="w-4 h-4 text-pink-400" />
            <span>Mathematical Model & Parameters</span>
          </h2>

          <div className="space-y-2 text-xs bg-gray-900/60 p-3 rounded-lg border border-gray-800 font-mono text-gray-300">
            <div>Sample: x = 1.0, y = 2.0</div>
            <div>Activation: h = tanh(w·x + b)</div>
            <div>Prediction: y_hat = v·h + c</div>
            <div>Loss: J = 0.5 · (y_hat - y)²</div>
            <div className="pt-2 border-t border-gray-800 text-pink-400">
              Init: w=0.5, b=0, v=1, c=0, η=0.1
            </div>
          </div>

          <button className="w-full py-2.5 px-4 rounded-lg bg-pink-600 hover:bg-pink-500 font-semibold text-xs text-white shadow-lg shadow-pink-600/20 flex items-center justify-center gap-2 transition-all">
            <Play className="w-4 h-4 fill-current" />
            <span>Run C++ Gradient Check & 100 Steps</span>
          </button>
        </div>

        {/* Right: Validation & Loss Curve Placeholder */}
        <div className="lg:col-span-2 space-y-6">
          <div className="p-5 rounded-xl bg-[#0f172a]/60 border border-gray-800 min-h-[300px] flex flex-col justify-between">
            <div className="flex items-center justify-between pb-3 border-b border-gray-800">
              <div className="text-xs font-bold text-white flex items-center gap-2">
                <TrendingDown className="w-4 h-4 text-pink-400" />
                <span>100-Step Training Loss Curve J(θ)</span>
              </div>
              <span className="text-[11px] font-mono text-gray-400">Learning Rate η = 0.1</span>
            </div>

            <div className="flex-1 flex flex-col items-center justify-center py-12 text-center">
              <Binary className="w-12 h-12 text-gray-700 mb-2 animate-pulse" />
              <div className="text-xs font-medium text-gray-400">Ready for Phase G6 execution</div>
              <p className="text-[11px] text-gray-500 max-w-sm mt-1">
                Will display analytical vs central finite difference gradients (ε = 10⁻⁶) and 100-step training loss history.
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

